#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
// 1. 聚合类型的新定义
// 1）没有任何构造函数，即使是用户声明的，包含=default
struct A {
    A() = default;
};

struct B {
};

// 2) 不允许有私有或者保护的非静态类型
struct C {
  private:
    int c;
};

struct D {
  protected:
    int d;
};
// 非静态类型不限制
struct E {
  private:
    static inline int e = 10;
};

// 3) 没有虚机制（虚继承和虚函数）
// 4) 必须是公开继承，基类是否是聚合类型没关系
struct F : public E {
    int f;
};

struct G : public C {
    int g;
};

// 2. 聚合类型可以直接初始化
// 虽然string不是聚合类型，但现在公开继承之后是满足聚合类型的
// 我们不需要创建构造函数可以直接初始化了，除了使用{}还可以使用()
class MyStringWithIndex : public std::string {
  public:
    int index_ = 0;
};

std::ostream &operator<<(std::ostream &o, const MyStringWithIndex &s) {
    o << s.index_ << ":" << s.c_str();
    return o;
}

// 3. 联合类型成员限制拓宽
// 过去联合类型的成员不能是一个非平凡类型，现在不再约束成员类型
// 但是如果联合类型存在非平凡类型，那么联合类型的特殊函数会被删除
// 所以我们要提供自己的析构和构造
union U {
    U() : x3() {
    }
    // 这里的析构必须手动调用，因为联合类型并不知道当前激活的成员是什么
    // 但这里有问题，如果使用的x4，那么由于x4没有初始化和正确析构所以会发生问题
    ~U() {
        x3.~basic_string();
    };

    int x1;
    float x2;
    std::string x3;
    std::vector<int> x4;
};

// 最好的方式使用空析构和构造
union U1 {
    U1() {};
    ~U1() {};
    int x1;
    float x2;
    std::string x3;
    std::vector<int> x4;
};

int main(void) {
    // 使用=default所以不是聚合类型
    std::cout << std::is_aggregate_v<A> << "\n";
    std::cout << std::is_aggregate_v<B> << "\n";
    // 含有私有类型
    std::cout << std::is_aggregate_v<C> << "\n";
    // 含有保护类型
    std::cout << std::is_aggregate_v<D> << "\n";
    std::cout << std::is_aggregate_v<E> << "\n";
    // 公开继承
    std::cout << std::is_aggregate_v<F> << "\n";
    std::cout << std::is_aggregate_v<G> << "\n";

    // 现在除了使用{}还可以使用()
    MyStringWithIndex s1{"hello", 2};
    MyStringWithIndex s2("hello", 2);
    std::cout << s1 << "\n";
    std::cout << s2 << "\n";

    // 这里我们使用placement new实现
    U1 u;
    auto x3 = new (&u.x3) std::string("hello world");
    std::cout << u.x3 << "\n";
    std::cout << *x3 << "\n";
    u.x3.~basic_string();

    auto x4 = new (&u.x4) std::vector<int>;
    u.x4.push_back(58);
    std::cout << u.x4[0] << "\n";
    std::cout << x4->at(0) << "\n";
    u.x4.~vector();

    return 0;
}
