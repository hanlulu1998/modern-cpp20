#include <ios>
#include <iostream>

// 1. noexcept替代throw()
int foo() noexcept {
    return 42;
}

int foo1() {
    return 42;
}

// c++20 throw()已经被移除了
int foo2() throw() {
    return 42;
}

// 3. 结合noexcept和noexcept()操作符
template <class T>
T copy(const T &o) noexcept(noexcept(T(o))) {
}

struct X {
};

#define PRINT_NOEXCEPT(x) \
    std::cout << #x << " = " << x << std::endl

struct M {
    M() {
    }
    M(const M &) {
    }
    M(M &&) noexcept {
    }
    M operator=(const M &) noexcept {
        return *this;
    }
    M operator=(M &&) {
        return *this;
    }
};

struct X1 {
    M m;
};

struct N {
    ~N() noexcept(false) {
    }
};

struct X2 {
    ~X2() noexcept(false) {
    }
};

struct X3 {
    N n;
};

// 6. noexcept签名
// 1）非noexcept函数能接受noexcept函数
using fp = void (*)();
void bar() noexcept {
}

// 2）虚函数也是如此，支持noexcept重写非noexcept
class Base {
  public:
    virtual void foo() {
    }
};
class Derived : public Base {
  public:
    void foo() noexcept override {};
};

// 3）模板不再兼容
void g1() noexcept {
}
void g2() {
}

template <class T1, class T2>
void zoo(T1 *, T2 *) {
}

int main(void) {
    // 2. noexcept运算符判断是否可能会异常
    std::cout << std::boolalpha;
    std::cout << "noexcept(foo())  = " << noexcept(foo()) << std::endl;
    std::cout << "noexcept(foo1()) = " << noexcept(foo1()) << std::endl;
    std::cout << "noexcept(foo2()) = " << noexcept(foo2()) << std::endl;

    // 如果noexcept函数发生异常，不会调用std::unexpected，而是调用std::terminate结束程序

    // 4. 默认构造和赋值都是noexcept的
    {
        X x;
        std::cout << std::boolalpha;
        PRINT_NOEXCEPT(noexcept(X()));
        PRINT_NOEXCEPT(noexcept(X(x)));
        PRINT_NOEXCEPT(noexcept(X(std::move(x))));
        PRINT_NOEXCEPT(noexcept(x.operator=(x)));
        PRINT_NOEXCEPT(noexcept(x.operator=(std::move(x))));
    }
    // 如果对应的函数在类型的基类和成员中也要具有noexcept声明，否则会失去noexcept声明
    {
        X1 x1;
        std::cout << std::boolalpha;
        PRINT_NOEXCEPT(noexcept(X1()));
        PRINT_NOEXCEPT(noexcept(X1(x1)));
        PRINT_NOEXCEPT(noexcept(X1(std::move(x1))));
        PRINT_NOEXCEPT(noexcept(x1.operator=(x1)));
        PRINT_NOEXCEPT(noexcept(x1.operator=(std::move(x1))));
        // 自定义实现的函数默认也不会带有noexcept声明
    }
    // 5. 类型的析构函数以及delete运算符默认带有noexcept声明，析构会影响delete
    // 即使自定义实现的析构函数也会默认带有noexcept声明，除非类型本身或者其基类和成员明确使用noexcept(false)声明析构函数
    {
        auto x = new X;
        auto x1 = new X1;
        auto x2 = new X2;
        auto x3 = new X3;

        std::cout << std::boolalpha;
        PRINT_NOEXCEPT(noexcept(x->~X()));
        PRINT_NOEXCEPT(noexcept(x1->~X1()));
        PRINT_NOEXCEPT(noexcept(x2->~X2()));
        PRINT_NOEXCEPT(noexcept(x3->~X3()));
        PRINT_NOEXCEPT(noexcept(delete x));
        PRINT_NOEXCEPT(noexcept(delete x1));
        PRINT_NOEXCEPT(noexcept(delete x2));
        PRINT_NOEXCEPT(noexcept(delete x3));
    }

    // 6. noexcept现在是函数签名的一部分
    // c++规定函数名总是隐式转换为函数指针，与数组不同
    fp func = bar;
    fp func2 = &bar;

    zoo(g1, g2);

    return 0;
}
