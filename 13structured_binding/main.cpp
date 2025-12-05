#include <cstddef>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

auto get_tuple() {
    return std::tuple(11, 12);
}

struct BindTest {
    int a = 1;
    std::string b = "Hello World";
};

class BindBase1 {
  public:
    int a = 42;
    double b = 11.7;
};

class Bind {
    int a = 1;

  public:
    double b = 1.0;
};

class BindTest1 : public BindBase1 {};

class BindBase2 {};

class BindTest2 : public BindBase2 {
  public:
    int a = 42;
    double b = 11.7;
};

class BindBase3 {
  public:
    int a = 42;
};

class BindTest3 : public BindBase3 {
  public:
    double b = 11.7;
    template <size_t Idx>
    auto &get() = delete;
};

template <>
auto &BindTest3::get<0>() {
    return a;
}

template <>
auto &BindTest3::get<1>() {
    return b;
}

namespace std {
template <>
struct tuple_size<BindTest3> {
    static constexpr size_t value = 2;
};

template <>
struct tuple_element<0, BindTest3> {
    using type = int;
};

template <>
struct tuple_element<1, BindTest3> {
    using type = double;
};
} // namespace std

// 使用外部get
// ```cpp
// template <size_t Idx>
// auto &get(BindTest3 &bt) = delete;

// template <>
// auto &get<0>(BindTest3 &bt) {
//     return bt.a;
// }

// template <>
// auto &get<1>(BindTest3 &bt) {
//     return bt.b;
// }
// ```

struct A {
    friend void foo();

  private:
    int i = 10;
};

void foo() {
    A a{};
    // 结构化绑定的限制不再强调必须为公开数据成员
    // 编译器会根据当前操作的上下文来判断是否允许结构化绑定
    auto [x] = a;
    std::cout << x << "\n";
}

int main(void) {
    // 1. 结构化绑定定义
    // 1)传统化方式，使用tie
    int x = 0, y = 0;
    std::tie(x, y) = get_tuple();
    std::cout << x << ", " << y << "\n";
    // 2）结构化绑定新方式
    auto [m, n] = get_tuple();
    std::cout << m << ", " << n << "\n";

    std::vector<BindTest> bt{{11, "hello"}, {12, "world"}, {13, "!!!"}};
    for (const auto &[x, y] : bt) {
        std::cout << x << ", " << y << "\n";
    }
    // 结构化绑定本质上是创建一个匿名副本，然后让绑定的变量成为访问成员的别名
    // 这里的别名是真的别名，不是所谓的引用
    // ```cpp
    // BindTest bt;
    // const auto & _anonymous = bt;
    // aliasname x = _anonymous.a
    // aliasname y = _anonymous.b
    // ```
    // 这里注意，结构化绑定前面的auto限定符会直接引用到匿名副本上
    // 所以绑定的变量能否修改原变量得看auto限定符

    // 3）忽略绑定对象
    // tie中可以使用ignore
    std::tie(x, std::ignore) = get_tuple();
    // 结构化绑定没有这种特性，但是我们可以自己手动忽略，但是无法在同一作用域重复使用
    auto [z, _] = get_tuple();

    // 2.结构化绑定的三种类型
    // 1）原生数组
    {
        int a[]{1, 3, 5};
        auto [x, y, z] = a;
        std::cout << x << ", " << y << ", " << z << "\n";
    }
    // 2）类
    {
        BindTest1 bt1;
        BindTest2 bt2;
        BindTest3 bt3;
        Bind b;
        // 不能有私有变量
        // auto [x, y] = b;
        auto [x1, y1] = bt1;
        auto [x2, y2] = bt2;
        // 成员变量不能分散在基类和派生类中，要么都在基类中，要么都在派生类中
        // auto [x3, y3] = bt3;
    }
    // 3）元组和类元组
    {
        // 目标类型提供std::tuple_size、std::tuple_element以及get的特化或者偏特化版本，即可实现结构化绑定
        // STL中std::pair和std::array都实现了以上函数，所以可以直接使用结构化绑定
        std::map<int, std::string> id2str{{1, "hello"}, {2, "world"}, {3, "!!!"}};
        for (const auto &[id, str] : id2str) {
            std::cout << id << ": " << str << "\n";
        }

        // 我们为BindTest3实现了结构化绑定的函数，现在可以使用了
        BindTest3 bt3;
        auto [x, y] = bt3;
    }

    // 3. 结构化绑定限制放松
    foo();

    return 0;
}
