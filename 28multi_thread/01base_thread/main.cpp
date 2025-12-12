#include <chrono>
#include <functional>
#include <iostream>
#include <stop_token>
#include <string>
#include <string_view>
#include <thread>
using namespace std::chrono_literals;
using namespace std::string_literals;

#define CLASS_LIFETIME_INFO(func_name)      \
    std::cout << std::this_thread::get_id() \
              << ": " << #func_name << "@" << this << "\n"

struct A {
    int m = -1;
    A(int m) : m(m) {
        std::cout << m << ")";
        CLASS_LIFETIME_INFO(A(int));
    }

    ~A() {
        std::cout << m << ")";
        CLASS_LIFETIME_INFO(~A());
    }

    A(const A &a) {
        m = a.m;
        std::cout << m << ")";
        CLASS_LIFETIME_INFO(A(const A &));
    }

    A(A &&a) {
        m = a.m;
        std::cout << m << ")";
        CLASS_LIFETIME_INFO(A(A &&));
    }
};

int main(void) {

    // 1. 经典thread
    auto work = [](std::string_view id, int time) {
        std::this_thread::sleep_for(std::chrono::milliseconds(time));
        std::cout << id << " use " << time << "ms\n";
    };
    auto t1 = std::thread(work, "w1"s, 500);

    // 1）join阻塞，主线程等待子线程完成
    t1.join();
    // 2）detach分离，子线程成为守护线程，c++运行时负责
    // 看不到输出是正常的，因为主线程结束了，打印输出关联的是主线程，所以没有输出
    // t1.detach();

    // 3）thread的参数拷贝与生命周期问题
    // a.主线程负责拷贝，不管什么参数都是退化拷贝临时值
    // 退化拷贝：去除cv，去除引用，退化指针，仅保留值
    // b.移动值到thread内部的tuple中
    // c.子线程调用apply(f,std::move(tuple))这里的调用也是移动，将值移动到函数内部
    std::cout << "parent thread id: " << std::this_thread::get_id() << "\n";
    auto lifetime_check = [](A &&a) {
        std::cout<<&a<<"@<";
        std::cout << "child thread id: " << std::this_thread::get_id() << "\n"; };
    A d{0};
    auto t2 = std::thread(lifetime_check, d);
    t2.join();
    // 从运行结果可以看到主线程负责了参数拷贝到thread的tuple的前期工作,以及A(0)临时变量的析构
    // 子线程负责了从tuple拿到参数进行函数调用以及tuple析构的所有工作

    // 传入引用时的thread
    std::cout << "parent thread id: " << std::this_thread::get_id() << "\n";
    auto lifetime_check2 = [](A &&a) {
        std::cout<<&a<<"@<";
        std::cout << "child thread id: " << std::this_thread::get_id() << "\n";a.m = 1; };
    A a{0};
    // 这里不论是传入ar还a都会发生退化拷贝，函数修改的还是临时值副本
    // 不会改变原始值的大小
    A &ar = a;
    auto t3 = std::thread(lifetime_check2, ar);
    t3.join();
    std::cout << "current a: " << a.m << "\n";
    // 如果想改变原始值那么必须使用指针或者std::ref模板
    // 使用std::ref会产生一个引用的包装结构，可复制，原理是通过指针返回引用
    // 但是这里的函数需要修改，不能使用右值引用，因为右值引用无法绑定左值
    auto lifetime_check3 = [](A &a) {
        std::cout<<&a<<"@<";
        std::cout << "child thread id: " << std::this_thread::get_id() << "\n";a.m = 1; };

    auto t4 = std::thread(lifetime_check3, std::ref(a));
    t4.join();
    std::cout << "current a: " << a.m << "\n";

    // 2. jthread
    // 1）自动join的，带有stop机制的thread
    {
        auto work = [](std::stop_token st, int id) {
            while (!st.stop_requested()) {
                std::cout << "Task " << id << " running\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        };

        std::jthread t(work, 42);
        std::this_thread::sleep_for(1s);
        t.request_stop();
    }
    // 2) 可回调
    {
        std::stop_source src;
        std::stop_token token = src.get_token();

        std::stop_callback cb(token, [] {
            std::cout << "callback: stop_requested!\n";
        });

        auto work = [](std::stop_token st, int id) {
            while (!st.stop_requested()) {
                std::cout << "Task " << id << " running\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
        };

        std::jthread t(work, token, 42);
        std::this_thread::sleep_for(1s);

        src.request_stop();
        // 这里不能使用线程自己的方法，它调用的是自己内部的stop_source产生的token，不是传入的token
        // t.request_stop();
    }

    std::cout << "main over\n";

    return 0;
}
