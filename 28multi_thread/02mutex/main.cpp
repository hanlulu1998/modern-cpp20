#include <chrono>
#include <iostream>
#include <locale>
#include <mutex>
#include <shared_mutex>
#include <thread>
using namespace std::chrono_literals;

// 可重入互斥量
std::recursive_mutex rmtx;

// 不可重入互斥量
std::mutex mtx;

// 带有时间的互斥量
std::timed_mutex tmtx;

// 共享的互斥量
std::shared_mutex smtx;

void rfunc(int n) {
    std::lock_guard lg(rmtx);
    std::cout << n << " ";
    if (n > 0)
        rfunc(n - 1);
    else
        std::cout << "\n";
}

int value = 0;
void reader() {
    // 多个读可以并行
    std::shared_lock lock(smtx); // 共享读
    std::cout << "read value = " << value << "\n";
}

void writer() {
    // 单个写独占
    std::unique_lock lock(smtx); // 独占写
    value++;
}

std::mutex m1, m2;
int x = 0, y = 0;

void worker1() {
    std::scoped_lock lock(m1, m2); // 一次锁多个，避免死锁
    x++;
    y++;
}

void worker2() {
    std::scoped_lock lock(m1, m2);
    x++;
    y++;
}

std::once_flag flag;
int resource = 0;

void init_resource() {
    std::call_once(flag, []() {
        resource = 100;
        std::cout << "init resource\n";
    });

    std::cout << "use resource = " << resource << "\n";
}

int main(void) {

    // 1. lock_guard
    // 不可手动开关锁，只能依靠RAII
    {
        std::lock_guard lg(mtx);
    }

    // 2. unique_lock
    // 可以手动开关锁，支持try_lock更加灵活
    // 性能更差
    {
        // 先不加锁
        std::unique_lock ul(mtx, std::defer_lock);
        ul.lock();

        ul.unlock();
        // 即使忘记unlock，也会自动unlock
    }

    // 3. 递归调用使用可重入mutex
    rfunc(3);

    // 4. 带有时间的互斥量，规定时间内请求上锁，超时放弃
    {
        if (tmtx.try_lock_for(1s)) {
            std::cout << "locked\n";
            tmtx.unlock();
        } else {
            std::cout << "timeout\n";
        }
    }

    // 5. 多读单写
    //
    std::thread t1(reader);
    std::thread t2(reader);
    std::thread t3(writer);

    t1.join();
    t2.join();
    t3.join();

    // 6. 多个mutex锁
    {
        // 两个锁时会触发问题
        // 线程A：1锁住，2没锁
        // 线程B：2锁住，1没锁
        // AB相互等待，然后卡主
        // scoped_lock可解决这个死锁问题
        std::thread t1(worker1);
        std::thread t2(worker2);

        t1.join();
        t2.join();

        std::cout << x << ", " << y << "\n";
    }

    // 7. 只调用一次
    // 适合在线程开始进行初始化
    // 可用于单例模式，不过现在c++保证了静态的安全初始化访问，不再需要这种double check和call_once
    {
        std::thread t1(init_resource);
        std::thread t2(init_resource);
        std::thread t3(init_resource);

        t1.join();
        t2.join();
        t3.join();
    }

    return 0;
}
