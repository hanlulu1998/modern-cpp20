#include <algorithm>
#include <iostream>
#include <memory>
#include <utility>
#include <vector>

// 智能指针通用操作：get获取原始指针，reset重置指针

struct A {
    ~A() {
        std::cout << "~";
    }
};

void del_func(A *ptr) {
    delete[] ptr;
    std::cout << "delete []\n";
}

void del_info(int *ptr) {
    std::cout << "delete: " << *ptr << "\n";
    delete ptr;
}

using del_func_t = void (*)(A *ptr);

struct C : public std::enable_shared_from_this<C> {
    // c++标准方式
    auto get_self() {
        return shared_from_this();
    }
    // 这种方式严重错误
    // 1）生命周期结束触发delete this，栈对象会直接崩溃
    // 2）如果对象本身被shared_ptr管理，就会出现一个this多个shared_ptr管理，double free
    auto bad_get_self() {
        return std::shared_ptr<C>(this);
    }
};

int main(void) {
    // 1. unique_ptr独占指针，唯一所有权
    // 1）不可复制只可移动
    auto p1 = std::make_unique<int>(1);
    std::unique_ptr<int> p2{new int{2}};
    auto p3 = std::move(p1);
    if (p1) {
        std::cout << "p1 is vaild\n";
    } else {
        std::cout << *p3 << "\n";
    }
    // 不推荐使用这种方式，失去了指针的意义
    auto &p = p3;
    std::cout << "p3 ref: " << *p << "\n";

    // 2）自定义删除器
    // 如果类型写成A，这里会出现bug，默认删除器调用的是delete
    // delete只会调用一次析构，delete []会读取new []隐藏在内存头的个数信息，依次调用析构
    std::unique_ptr<A[]> p4{new A[3]};
    // 或者自定义删除
    auto del = [](A *ptr) { delete[] ptr;std::cout<<"\n"; };
    std::unique_ptr<A, decltype(del)> p5(new A[3], del);

    std::unique_ptr<A, del_func_t> p6(new A[3], del_func);

    // 3) 尺寸问题
    // 因为删除器是unique_ptr的类型组成，所以不同删除器的unique_ptr不同，不能算作同类型
    // 默认unique_ptr大小与裸指针一致，但是如果删除器是lambda这种匿名对象，大小不变，
    // 如果是函数尺寸就是UB，可能变大也可能不变
    std::cout << "p4: " << sizeof(p4) << "\n";
    std::cout << "p5: " << sizeof(p5) << "\n";
    std::cout << "p6: " << sizeof(p6) << "\n";

    // 2. shared_ptr指针
    // 1）复制会增加引用计数
    // shared_ptr除了持有原始对象指针，还持有一个控制块指针，控制块包含强弱引用技术，以及一些其他数据如自定义删除器等等
    // 在复制时，这些指针会复制，大家指向同一份控制块
    // 但是在单独创建时，每个shared_ptr都会有自己的控制块，控制块不统一会触发double free等问题
    // 经典问题是将一个裸指针交给多个shared_ptr管理，会触发double free
    auto s = std::make_shared<int>(1);
    // 与独占指针不同，不需要填入模板参数，对于数组问题的析构和unique_ptr一样
    std::shared_ptr<int> s3(new int{11}, del_info);
    std::shared_ptr<int> s1(new int{10}, [](int *ptr) { del_info(ptr); });
    auto s2 = s3;
    std::cout << "s3 count: " << s3.use_count() << "\n";
    s3 = s1;
    std::cout << "s3 count: " << s3.use_count() << "\n";
    auto s4 = std::move(s2);
    std::cout << "s4 count: " << s4.use_count() << "\n";
    if (!s2) {
        std::cout << "s2 is invalid\n";
    }

    // 2) shared_ptr的尺寸问题，为裸指针的2倍
    // 指向同一个类型的shared_ptr是一个类型，即使他们的删除器不同
    std::cout << "s: " << sizeof(s) << "\n";
    std::cout << "s1: " << sizeof(s1) << "\n";
    std::cout << "s4: " << sizeof(s4) << "\n";

    // 容器使用值拷贝会增加引用计数
    std::vector vec{s, s1, s4};
    std::cout << "s count: " << s.use_count() << "\n";

    // 3) this 的shared_ptr
    auto c1 = std::make_shared<C>();
    auto c2 = c1->get_self();

    std::cout << "c2 count: " << c2.use_count() << "\n";

    // 3. weak_ptr弱指针
    // 1）绑定到shared_ptr不影响强引用计数，但本身的弱引用计数会影响
    // 用来监视shared_ptr的生命周期
    auto pi = std::make_shared<int>(2);
    std::weak_ptr<int> piw(pi);
    pi.reset();
    if (piw.expired()) {
        std::cout << "piw dead\n";
    }
    // lock获取shared_ptr，成功引用计数+1，否则返回nullptr
    if (piw.lock() == nullptr) {
        std::cout << "piw lock nullptr\n";
    }

    // 2）尺寸问题
    // weak_ptr同shared_ptr一样，都是裸指针的2倍

    // 3）弱指针解决循环引用
    // A中持有B的shared_ptr，B中也持有A的shared_ptr，两者互相持有，就会触发循环引用
    // 解决办法是让其中一个成为weak_ptr
    return 0;
}
