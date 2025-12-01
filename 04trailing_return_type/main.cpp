#include "type_name.hpp"
#include <iostream>
void foo() {
    std::cout << "foo\n";
}

// 1. 使用返回类型后置，可以不用定义void(*)()类型
auto bar() -> void (*)() {
    return foo;
}

// 2. 现在auto已经可以省略类型后置了
auto bar1() {
    return foo;
}

// 3. decltype(auto)也是类似用法，只不过推导不一样
decltype(auto) bar2() {
    return &foo;
}

// 4. 在模板中使用类型后置
template <class T1, class T2>
auto sum(T1 t1, T2 t2) {
    return t1 + t2;
}

int main(void) {
    std::cout << type_name_v<decltype(bar())> << "\n";
    std::cout << type_name_v<decltype(bar1())> << "\n";
    std::cout << type_name_v<decltype(bar2())> << "\n";
    std::cout << type_name_v<decltype(foo)> << "\n";

    auto a = bar();
    auto b = bar1();
    auto c = bar2();
    a();
    b();
    c();

    auto d = sum(1, 2);
    std::cout << d << "\n";

    // 5. lambda表达式
    auto f1 = [](auto a1, auto a2) { return a1 + a2; };
    std::cout << f1(1, 2) << "\n";

    auto f2 = [](int &i) -> auto & { return i; };
    // 使用declt(auto)和auto&一样
    auto f3 = [](int &i) -> decltype(auto) { return i; };
    int x1 = 1;
    // 这里不要使用auto，使用auto会忽略引用
    decltype(auto) x1_ref = f2(x1);
    x1_ref = 2;
    std::cout << x1_ref << "," << x1 << "\n";

    decltype(auto) x2_ref = f3(x1);
    x2_ref = 3;    
    std::cout << x2_ref << "," << x1 << "\n";

    return 0;
}
