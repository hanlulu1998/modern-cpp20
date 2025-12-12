#include <algorithm>
#include <chrono>
#include <coroutine>
#include <future>
#include <iostream>
#include <memory>
#include <thread>
using namespace std::chrono_literals;

// 1. co_await关键字
// 挂起协程 → 执行 awaiter 的三件套 → 恢复时获得结果
// 编译器内部展开如下
// ```
// auto awaiter = expr.operator co_await();
// if (!awaiter.await_ready()) {
//     awaiter.await_suspend(handle);
//     return;     // 挂起
// }
// auto result = awaiter.await_resume();
// ```

// 等待体需要实现await三件套函数
class Awaiter {
  public:
    Awaiter() = default;
    explicit Awaiter(int i) : i_(i) {
    }
    // await_ready() —— 是否需要挂起
    bool await_ready() {
        return false;
    }

    // await_suspend() —— 挂起时做什么（通常交给调度器）
    void await_suspend(std::coroutine_handle<> h) {
        std::cout << "Suspend\n";
        std::this_thread::sleep_for(500ms);
        // 退出挂起，恢复协程
        h.resume();
    }

    // await_resume() —— 恢复时返回值
    int await_resume() {
        return i_;
    }

  private:
    int i_ = 0;
};

// 等待器，实现co_wait重载即可
struct Awaitable {
    int value;
};
Awaiter operator co_await(Awaitable a) {
    return Awaiter{a.value};
}

// 2. 协程函数的返回类型必须是包含promise_type类型的类型
// 编译器生成promise_type对象，初始化协程，执行状态机，自动结果返回给调用者
struct Task {
    struct promise_type {
        Task get_return_object() {
            return {};
        }
        std::suspend_never initial_suspend() noexcept {
            return {};
        }
        std::suspend_never final_suspend() noexcept {
            return {};
        }
        void unhandled_exception() {
        }
        void return_void() {
        }
    };
};

Task foo() {
    int r = co_await Awaitable{5};
    std::cout << "result = " << r << "\n";
}

// 1) promise.initial_suspend()
// 2) 协程体执行
// 3) co_await → awaiter 逻辑
// 4) co_return → promise.return_value()
// 5) promise.final_suspend()
struct generator {
    struct promise_type {
        int *ptr_ = nullptr;

        auto get_return_object() {
            return generator{*this};
        }

        auto initial_suspend() noexcept {
            return std::suspend_always{};
        }

        // 这里切记，如果结束阻塞不挂起，协程会自动析构
        // next中的析构就是double free
        auto final_suspend() noexcept {
            return std::suspend_always{};
        }

        void unhandled_exception() {
        }

        auto yield_value(int &value) {
            ptr_ = &value;
            return std::suspend_always{};
        }
        // 无返回值，co_return;
        void return_void() {
        }
    };

    explicit generator(promise_type &p) : handle_(std::coroutine_handle<promise_type>::from_promise(p)) {
    }

    ~generator() {
        if (handle_) {
            handle_.destroy();
        }
    }

    int next() {
        if (!handle_) {
            return -1;
        }
        handle_.resume();
        if (handle_.done()) {
            handle_.destroy();
            handle_ = nullptr;
            return -1;
        }
        return *(handle_.promise().ptr_);
    };

    std::coroutine_handle<promise_type> handle_;
};

generator gen() {
    for (int i = 0; i < 10; ++i) {
        // 相当于 co_await promise.yield_value(i);
        co_yield i;
    }
}

struct genreturn {
    struct promise_type {
        int value_ = 0;
        auto get_return_object() {
            return genreturn{*this};
        }

        auto initial_suspend() noexcept {
            return std::suspend_never{};
        }

        auto final_suspend() noexcept {
            return std::suspend_always{};
        }

        void unhandled_exception() {
        }

        // 有返回值，和return_void互斥
        void return_value(int value) {
            value_ = value;
        }
    };

    explicit genreturn(promise_type &p) : handle_(std::coroutine_handle<promise_type>::from_promise(p)) {
    }

    ~genreturn() {
        if (handle_) {
            handle_.destroy();
        }
    }

    int get() {
        if (!ready_ && handle_) {
            value_ = handle_.promise().value_;
            ready_ = true;
            if (handle_.done()) {
                handle_.destroy();
                handle_ = nullptr;
            }
        }
        return value_;
    }

    std::coroutine_handle<promise_type> handle_;
    int value_ = 0;
    bool ready_ = false;
};

genreturn get_return() {
    co_return 5;
}

int main(void) {
    foo();
    auto obj = gen();
    for (int i = 0; i < 11; ++i) {
        std::cout << obj.next() << " ";
    }
    std::cout << "\n";

    auto re = get_return();
    std::cout << re.get() << "\n";
    std::cout << re.get() << "\n";
    std::cout << re.get() << "\n";

    return 0;
}
