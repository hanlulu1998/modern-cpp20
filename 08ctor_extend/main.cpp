#include <deque>
#include <iostream>
#include <list>
#include <type_traits>
#include <vector>
// 1. =delete和=default的使用
struct S1 {
    S1();
    // =delete删除该函数，不可在外部使用
    ~S1() = delete;
};

// =default使用默认的编译器选择，可以在外部使用
S1::S1() = default;

class S2 {
  private:
    int s;

    S2() = default;
    ~S2() = delete;
};

// 2. 用户提供和用户声明的区别
// 用户提供(user-provided)是用户自己声明并定义了函数，使用外部=default也属于用户提供
// 用户声明（user-declared）是用户只声明不定义，或者使用内部=delete和=default

// 4. 委托构造：构造函数可以使用另一个构造函数进行构造
// 1) 委托构造函数也可以是别人的代理构造函数
// 2）不可以循环委托，A(int)->A(int,int)->A(int)会发生崩溃
// 3）委托构造函数不能在初始化列表里对成员和基类进行初始化
// 4）委托构造函数的顺序是先执行代理构造函数的初始化列表，然后执行代理构造函数的主体，最后再执行委托构造函数的主体
// 5）如果代理构造函数已经执行完毕，委托构造函数主题异常，则会直接析构：因为代理构造完成后对象已经完成了
class D {
  public:
    D() : D(0) {
    }
    D(int a) : D(a, 0.) {
    }
    D(double b) : D(0, b) {
    }
    D(int a, double b) : a_(a), b_(b) {
        CommonInit();
    }

  private:
    void CommonInit() {
    }
    int a_;
    double b_;
};

// 5. 委托模板构造函数
class A {
    // template <class T>
    // A(T begin, T end) : l_(begin, end) {
    // }

    A(auto begin, auto end) : l_(begin, end) {
    }

    std::list<int> l_;

  public:
    A(std::vector<short> &v) : A(v.begin(), v.end()) {
    }
    A(std::deque<int> &v) : A(v.begin(), v.end()) {
    }
};

// 6. 使用Function-try-block捕获异常时，过程与初始化列表一样
// 但是如果异常在代理构造函数中抛出，那么委托构造函数主体则不再执行，而是去到catch中
class X {
  public:
    X()
    try : X(0) {
        std::cout << "X()\n";
    } catch (int e) {
        std::cout << "catch: " << e << std::endl;
        throw 3;
    }
    X(int a)
    try : X(a, 0.) {

        std::cout << "X(int a)\n";
    } catch (int e) {
        std::cout << "catch: " << e << std::endl;
        throw 2;
    }
    X(double b) : X(0, b) {

        std::cout << "X(double)\n";
    }
    X(int a, double b)
    try : a_(a), b_(b) {
        std::cout << "X(int a, double b)\n";
        throw 1;
    } catch (int e) {
        std::cout << "catch: " << e << std::endl;
        throw;
    }

  private:
    int a_;
    double b_;
};

// 7. 继承构造
class Base {
    Base(long) {
    }

  protected:
    Base(char c) {
    }

  public:
    Base() {
    }
    Base(int) {
    }
    explicit Base(double) {
    }

    Base(float) {
    }

    Base(const Base &) {
    }
};

class Derived : public Base {
  public:
    using Base::Base;
};

int main(void) {

    // 3. 平凡类型：对象的构造、复制、移动、销毁都可以用简单的内存操作 (memcpy、memmove、memset) 完成，不需要调用任何用户代码。
    // 1）类的6大函数都是平凡的（即用户声明或者默认）
    // 2）没有虚机制，即虚函数，虚基类
    // 3）基类和成员也是平凡的（引用类型不是平凡的，指针是的）

    // S1使用了外部=default构造，所以构造不是平凡的
    std::cout << std::is_trivial_v<S1> << "\n";
    // S2的6大函数都是平凡的，虽然析构是=delete，但是平凡不受=delete和私有性影响
    // 因为按照平凡定义，所有操作都可以使用基础的内存操作，不会调用用户代码
    std::cout << std::is_trivial_v<S2> << "\n";

    // 标准布局类型
    // 1）所有非静态成员具有相同的访问控制（全部 public 或全部 private 等）
    // 2）没有虚函数、没有虚基类
    // 3）所有非静态数据成员都是 standard-layout
    // 4）成员必须从当前类自身声明的第一个成员开始，而不能让基类的成员排在最前面
    std::cout << std::is_standard_layout_v<S1> << "\n";
    std::cout << std::is_standard_layout_v<S2> << "\n";
    // 原来的POD = 平凡+标准布局

    try {
        X x;
    } catch (int e) {
        std::cout << "catch: " << e << "\n";
    }

    // 7. 继承构造本质上是一种模板构造的机制，使用时产生，不使用时不产生
    Derived d1{1};
    // 1）不会提升属性，私有不会继承，protected依旧如此
    // Derived d2{'c'};
    // 2）继承构造不会影响默认构造的产生
    Derived d3;
    // 3）会继承explicit属性
    // d3 = 3.0;
    // 4）不会继承类的默认构造和复制构造
    d3 = 3.0f;
    // 5）继承构造不会继承默认参数

    return 0;
}
