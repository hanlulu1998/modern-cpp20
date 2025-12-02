#include "type_name.hpp"
#include <cstdio>
#include <iostream>

struct A {
    double x = 1.0;
    // 4.1)只能使用static inline和static const
    static inline auto AI = 10;
    static const auto SC = 10;
};

template <decltype(auto) N>
void f() {
    std::cout << type_name_v<decltype(N)> << "\n";
}

int main(void) {

    // 1. atuo推导规则，本质上是模板类型推导

    // 1）总是忽略顶层cv，保留底层cv
    // 顶层cv指的是修饰变量本身的，底层cv是修饰指针指向变量的
    printf("auto_rule1=============================\n");
    const volatile int i = 5;
    // 顶层cv被忽略
    auto auto_a = i;
    // 引用和指针时修饰该变量的cv变成了底层
    auto &auto_b = i;
    auto *auto_c = &i;
    std::cout << type_name_v<decltype(auto_a)> << "\n";
    std::cout << type_name_v<decltype(auto_b)> << "\n";
    std::cout << type_name_v<decltype(auto_c)> << "\n";

    // 底层cv保留
    const volatile int *i1;
    auto auto_a1 = i1;
    auto *auto_c1 = i1;

    std::cout << type_name_v<decltype(auto_a1)> << "\n";
    std::cout << type_name_v<decltype(auto_c1)> << "\n";

    // 顶层cv会被忽略
    int *const volatile i2{};
    auto auto_a2 = i2;
    auto *auto_c2 = i2;
    std::cout << type_name_v<decltype(auto_a2)> << "\n";
    std::cout << type_name_v<decltype(auto_c2)> << "\n";

    // 2)总是移除引用
    printf("auto_rule2=============================\n");
    // 和指针一样，引用的cv是继承自绑定的类型，
    // 但是和指针不同，引用本身没有cv属性，即使声明也没有意义
    const volatile int &j = i;
    auto auto_d = j;
    std::cout << type_name_v<decltype(auto_d)> << "\n";

    // 3)数组和函数总是衰变成指针，除非使用引用
    printf("auto_rule3=============================\n");
    const volatile int array[]{1, 2, 3};
    auto &auto_array_ref = array;
    auto auto_array_base = array;
    std::cout << type_name_v<decltype(auto_array_ref)> << "\n";
    std::cout << type_name_v<decltype(auto_array_base)> << "\n";

    // 4)万能引用，左值为左值引用，右值为右值引用
    printf("auto_rule4=============================\n");
    auto &&auto_rr1 = i;
    auto &&auto_rr2 = 5;
    std::cout << type_name_v<decltype(auto_rr1)> << "\n";
    std::cout << type_name_v<decltype(auto_rr2)> << "\n";

    // 5)初始化列表，使用{}，必须单元素才能为对应类型，
    // 使用={}，必须为多元组，才能初始化类型
    printf("auto_rule5=============================\n");
    auto auto_l1 = {1, 2, 3};
    auto auto_l2{1};
    std::cout << type_name_v<decltype(auto_l1)> << "\n";
    std::cout << type_name_v<decltype(auto_l2)> << "\n";

    // 6)表达式都是按值传递
    printf("auto_rule6=============================\n");
    auto auto_exp = (1);
    auto auto_exp1 = (i);
    auto auto_exp2 = (i1);
    auto auto_exp3 = (i2);
    auto auto_exp4 = (j);
    auto auto_exp5 = (array);

    std::cout << type_name_v<decltype(auto_exp)> << "\n";
    std::cout << type_name_v<decltype(auto_exp1)> << "\n";
    std::cout << type_name_v<decltype(auto_exp2)> << "\n";
    std::cout << type_name_v<decltype(auto_exp3)> << "\n";
    std::cout << type_name_v<decltype(auto_exp4)> << "\n";
    std::cout << type_name_v<decltype(auto_exp5)> << "\n";

    // 7)c++三元运算符常规类型提升，即使值为2，但是2.0为double
    // 所以auto依旧为double
    // 同样的情况出现在大多数运算符中
    printf("auto_rule7=============================\n");

    auto s = true ? 2 : 2.0;
    std::cout << type_name_v<decltype(s)> << "\n";

    // 2. decltype 推导规则
    // 1）默认保留所有cv和引用属性（顶层和底层都保留），同时不进行函数和数组的衰退
    printf("decltype_rule1=============================\n");
    std::cout << type_name_v<decltype(i)> << "\n";
    std::cout << type_name_v<decltype(i1)> << "\n";
    std::cout << type_name_v<decltype(i2)> << "\n";
    std::cout << type_name_v<decltype(j)> << "\n";
    std::cout << type_name_v<decltype(array)> << "\n";

    // 2)表达式如果是左值，则为T&，右值为T（纯右值）或者T&&（将亡值）
    printf("decltype_rule2=============================\n");
    std::cout << type_name_v<decltype((i))> << "\n";
    std::cout << type_name_v<decltype((i1))> << "\n";
    std::cout << type_name_v<decltype((i2))> << "\n";
    std::cout << type_name_v<decltype((j))> << "\n";
    std::cout << type_name_v<decltype((array))> << "\n";
    std::cout << type_name_v<decltype((10))> << "\n";
    std::cout << type_name_v<decltype((std::move(i)))> << "\n";

    printf("decltype_auto_rule=============================\n");
    // 3. decltype(auto) 用法和auto相同，推导使用decltype
    // 1) decltype(auto)可以作为非类型模版参数的占位符，推导规则一致
    static const int x = 10;
    static int y = 7;
    f<x>();
    f<(x)>();
    f<(y)>();

    printf("other_rule=============================\n");
    // 4. 其他细节

    // 1) 类中的非静态变量不可以使用auto
    // 非静态变量使用static const或者static inline

    // 2) 关于decltype成员变量的cv问题
    const A *a = new A;
    std::cout << type_name_v<decltype(a)> << "\n";
    std::cout << type_name_v<decltype(a->x)> << "\n";
    std::cout << type_name_v<decltype((a->x))> << "\n";

    return 0;
}
