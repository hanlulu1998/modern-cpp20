#include <chrono>
#include <future>
#include <iostream>
#include <stop_token>
#include <thread>
using namespace std::chrono_literals;
int main(void) {
    // 1. std::future异步过程的数据读取端
    // future 不可复制，只能移动
    // std::launch::async   // 一定在新线程执行
    // std::launch::deferred // 延迟执行：get() 时才真正运行
    // 无参数时默认是两者中自动选择最优
    {
        std::future<int> fut = std::async([] { return 42; });
        // get只能调用一次
        std::cout << fut.get() << "\n"; // 阻塞直到结果就位
    }
    // 2. std::promise异步过程中数据设置端
    // promise 不可复制，只能移动
    // set_value()
    // set_exception()
    // set_value_at_thread_exit()
    // promise和future内部维护一个共享状态，允许拷贝的话多次写入共享状态会发生竞态
    {
        std::promise<int> p;
        std::future<int> fut = p.get_future();

        // std::shared_future可以复制和多读
        // 一旦使用share()，原来的future就会被移动走
        auto fut1 = fut.share();
        auto fut2 = fut1;

        std::thread t([&] {
            std::this_thread::sleep_for(1s);
            p.set_value(123);
        });

        std::cout << fut.valid() << "\n"; // 123

        std::cout << fut1.get() << "\n";
        std::cout << fut2.get() << "\n";

        t.join();
    }

    // 3. packaged_task = 可调用对象 + promise + future 的封装
    // 可以移动，不可复制
    // 调用后自动把返回值写入 future
    // 适合放入线程池或手动调度
    {
        std::packaged_task<int(std::stop_token)> task([](std::stop_token st) {
            while (!st.stop_requested()) {
                std::cout << "wait for stop\n";
            }
            return 10;
        });

        std::future<int> fut = task.get_future();
        // std::stop_source ss;
        // auto token = ss.get_token();
        std::jthread t(std::move(task)); // 执行任务
        std::this_thread::sleep_for(1s);
        t.request_stop();
        t.join();
        std::cout << fut.get() << "\n"; // 10
    }

    return 0;
}
