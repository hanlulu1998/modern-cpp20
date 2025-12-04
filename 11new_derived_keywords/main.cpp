#include <iostream>

// 1. 三个over的含义
// 1）重写（override）：派生类覆盖基类虚函数
// 2）重载（overload）：函数名相同，但函数参数不同
// 3）隐藏（overwrite）：只要函数名相同，基类函数就会被隐藏

// 2. override函数，保证继承类的虚函数被重写，让编译器检查
class Base {
  public:
    virtual void some_func() {
    }
    virtual void foo(int x) {
    }
    virtual void bar() const {
    }
    void baz() {
    }
};

class Derived : public Base {
  public:
    virtual void some_func() override {
    }
    // final保证改函数不能重写
    virtual void foo(int x) override final {
    }
    virtual void bar() const override {
    };

    // virtual void sone_func() override {
    // }
    // virtual void foo(int &x) override {
    // }
    // virtual void bar() override {
    // }
    // virtual void baz() override {
    // }
};

// 3. final关键字，保证类和函数不被继承和重写
class Derived2 : public Derived {
  public:
    // 不能被重写
    // void foo(int x) {
    // }
};

int main(void) {

    return 0;
}
