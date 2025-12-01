#include "type_name.hpp"
#include <iostream>
#include <string>
#include <utility>

template <class T>
void bar1(T &&t) {
    std::cout << t << "\n";
}

void bar2(auto &&t) {
    std::cout << t << "\n";
}

struct A {
    char a;
    char b;

    A() {
        std::cout << "ctor" << std::endl;
    }

    A(char a, char b) : a(a), b(b) {
        std::cout << "ctor" << std::endl;
    }
};

A func() {
    return {'a', 'b'};
}

template <class T>
void show_type(T && t) {
    // 这里不能使用T t，类似于auto t，会移除cv属性和引用
    std::cout << type_name_v<decltype(t)> << "\n";
}

template <class T>
void perfect_forwarding(T &&t) {
    show_type(std::forward<T>(t));
}

int main(void) {

    // 1.  c++值类型如下：
    //////////////////////////////
    //       expression         //
    //         /    \           //
    //     glvalue   rvalue     //
    //     /     \  /    \      //
    //  lvalue xvalue<--prvaule //
    //////////////////////////////

    // 1）左值与之前的左值含义一致，可取地址
    // 2）纯右值就是之前的右值，临时对象，字面量，返回值（临时值，没有身份）
    // 注意：字符串字面量是左值，因为在内存中一直有这个值，固定位置可取地址
    // 3）将亡值属于新定义值，就是即将过期的值，表示资源可以被重用的对象
    // 产生将亡值的两种方式：a.类型转换（std::move,static_cast等）；b.临时量实质化
    // 临时量实质化：每当纯右值出现在泛左值的位置上，就会发生如此转换

    // 左值引用
    A c{'a', 'b'};
    A &c1 = c;
    std::cout << sizeof(c1) << "\n";

    // 常量左值引用
    const A &c2 = A{'a', 'b'};
    std::cout << sizeof(c2) << "\n";
    // 右值引用
    A &&c3 = A{'a', 'b'};
    std::cout << sizeof(c3) << "\n";

    // 临时量实质化
    std::cout << func().a << "\n";

    // 2. 类的默认函数生成口诀
    // 1）只要有构造（=default）就不生成默认构造
    // 2）有任意一个copy就不生成2个默认move
    // 3）有任意一个move就不生成2个默认copy
    // 4）自定义析构不再影响copy和move的生成
    // 5）如果成员不能复制和移动，整个类就不能复制和移动

    // 3. 万能引用和引用折叠
    int a1 = 1;
    int &a2 = a1;
    int &&a3 = 1;

    // 传统模板方式
    bar1(a1);
    bar1(a2);
    bar1(a3);

    // auto&&方式
    bar2(a1);
    bar2(a2);
    bar2(a3);
    std::cout << "==================================\n";
    std::cout << type_name_v<decltype(a1)> << "\n";
    std::cout << type_name_v<decltype(a2)> << "\n";
    std::cout << type_name_v<decltype(a3)> << "\n";

    // 只要用左值初始化就是左值引用
    // 只要是右值初始化就是右值引用
    // 不论如何都是引用类型
    // 引用折叠，对于T &,T &&,T的组合，只要有T &参与最后就是T &，其他组合都是T &&

    auto &&b1 = 1;
    auto &&b2 = a1;
    auto &&b3 = a2;
    auto &&b4 = a3;

    std::cout << "==================================\n";
    std::cout << type_name_v<decltype(b1)> << "\n";
    std::cout << type_name_v<decltype(b2)> << "\n";
    std::cout << type_name_v<decltype(b3)> << "\n";
    std::cout << type_name_v<decltype(b4)> << "\n";

    std::cout << "==================================\n";
    int s = 10;
    int &s1 = s;
    // 完美转发
    perfect_forwarding(s);
    perfect_forwarding(s1);
    perfect_forwarding(10);
    return 0;
}
