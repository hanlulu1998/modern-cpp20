#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <latch>
#include <mutex>
#include <semaphore>
#include <thread>
using namespace std::chrono_literals;

std::mutex m;
std::condition_variable cv;
bool ready = false;

void wait_thread() {
    // 必须使用uniqu_lock，需要自动解锁
    std::unique_lock<std::mutex> lock(m);
    cv.wait(lock, [] { return ready; }); // 自动 unlock → sleep → lock
    std::cout << "Ready!\n";
}

void signal_thread() {
    {
        std::lock_guard<std::mutex> lock(m);
        std::this_thread::sleep_for(1s);
        ready = true;
    }
    cv.notify_one(); // 或 notify_all()
}

std::atomic<bool> ready1 = false;

void waiter() {
    bool expected = false;
    ready1.wait(expected); // 当ready等于expected时阻塞
    std::cout << "ready!\n";
}

void notifier() {
    std::this_thread::sleep_for(1s);
    ready1.store(true);
    // 阻塞后必须由notify唤醒
    ready1.notify_one();
}

std::barrier sync_point(3); // 需要 3 个线程到达

void worker1(int id) {
    for (int i = 0; i < 3; ++i) {
        std::cout << "Thread " << id << " reached barrier " << i << "\n";
        sync_point.arrive_and_wait();
    }
}

int main(void) {
    // 1. 条件变量
    {
        std::thread t1(wait_thread);
        std::thread t2(signal_thread);
        t1.join();
        t2.join();
    }

    // 2. atomic.wait和automic.notify
    // 更加轻量，性能更好，无锁
    {

        std::thread t1(waiter);
        std::thread t2(notifier);
        t1.join();
        t2.join();
    }

    // 3. latch 单次同步
    {
        // 不用手动再写count_down
        std::latch l(3);
        auto worker = [&](int i) {
            std::cout << "count down: " << i << "\n";
            l.count_down();
        };
        std::thread a(worker, 1), b(worker, 2), c(worker, 3);
        l.wait();
        a.join();
        b.join();
        c.join();
    }

    // 4. barrier 可重复的同步
    // 所有线程到达统一阶段，然后一起继续
    {
        std::thread t1(worker1, 1);
        std::thread t2(worker1, 2);
        std::thread t3(worker1, 3);

        t1.join();
        t2.join();
        t3.join();
    }

    // 5. 信号量
    {
        // 1）限制并发的数量
        // 这里的模板形参为最大计数，影响release
        // 构造形参为启动时的可用资源数量，影响的是acquire
        std::counting_semaphore<3> sem(3);
        auto worker = [&](int id) {
            // 计数减1，<=0时阻塞
            sem.acquire(); // 尝试进入
            std::cout << "Thread " << id << " working\n";

            std::this_thread::sleep_for(std::chrono::seconds(1));

            std::cout << "Thread " << id << " done\n";
            // 计数+1，但是不能超过3
            sem.release(); // 释放一个名额
        };

        std::thread ts[6];
        for (int i = 0; i < 6; ++i)
            ts[i] = std::thread(worker, i);

        for (auto &t : ts)
            t.join();

        // 2) 一次性通知事件
        // 初始时为阻塞
        std::binary_semaphore bsem(0);
        auto waiter = [&]() {
            std::cout << "Waiting...\n";
            bsem.acquire(); // 等待通知
            std::cout << "Go!\n";
        };

        std::thread t(waiter);
        std::this_thread::sleep_for(std::chrono::seconds(2));

        bsem.release(); // 唤醒
        t.join();
    }
    return 0;
}
