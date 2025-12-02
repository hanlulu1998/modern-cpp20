#include <cstdio>
#include <iostream>
#include <vector>
int a = -10;

void foo1(void (*)()) {
}

void foo2(void (&)()) {
}

int main(void) {

    // 1. 值捕获默认不可变，引用捕获可变
    // 捕获的值只能是当前作用域的自动存储类型，即栈类型数据
    // 换句话说，如果lambda定义在全局，那就不能捕获任何值，因为全局不存在自动存储类型
    int b = 10;

    // 在有mutbale的地方，()不可省略
    auto f = [b]() mutable {
        b += 5;
        // 对于全局变量像普通函数一样都可以使用
        a = -1;
        return b;
    };
    printf("=================================\n");
    std::cout << f() << "\n";
    b = 0;
    // 注意：lambda本质上是构造一个实现()操作符的类，是一个语法糖
    // 在定义lambda函数捕获值时，就相当于调用构造，所以一旦捕获完成后续就无关了
    // 并且捕获变量是内部变量，会持久化
    std::cout << f() << "\n";
    std::cout << a << "\n";
    printf("=================================\n");

    b = 10;
    auto f_copy = f;
    printf("=================================\n");
    std::cout << f_copy() << "\n";
    b = 0;
    std::cout << f_copy() << "\n";
    std::cout << a << "\n";
    printf("=================================\n");

    // 引用可以修改捕获的值
    b = 10;
    auto f1 = [&b]() {
        b += 5;
        a = -2;
        return b;
    };

    printf("=================================\n");
    std::cout << f1() << "\n";
    b = 0;
    std::cout << f1() << "\n";
    std::cout << a << "\n";
    printf("=================================\n");

    b = 10;
    // 2. 特殊捕获
    // =捕获所有值，包含this
    // &捕获所有引用，包含this
    // 广义捕获
    auto f2 = [c = b + 1]() mutable {
        c += 5;
        return c;
    };

    printf("=================================\n");
    std::cout << f2() << "\n";
    b = 0;
    std::cout << f2() << "\n";
    std::cout << a << "\n";
    printf("=================================\n");

    // [=,this]等同于之前的[=]，现在[=]已经不支持隐式不会this了
    // [=,*this]用于捕获this的拷贝，在函数体内可以像以前一样使用this

    // 3. lambda模板
    auto f3 = []<typename T>(T t) {
        std::cout << t << "\n";
    };

    printf("=================================\n");
    f3(1);
    printf("=================================\n");

    // 4. 无状态lambda优化
    auto f4 = []() {};
    foo1(f4);
    foo2(*f4);

    // 5. 无捕获lambda和普通类完成一致，有默认构造和赋值
    std::vector v1{f4}, v2{f4};
    v1 = v2;
    v1 = std::move(v2);

    // 6.有捕获lambda分情况：
    // 默认构造和赋值删除
    // 拷贝构造和移动构造都会默认生成
    auto f_move = std::move(f);
    auto f1_1 = f1;
    auto f1_2 = f1;
    // f1_2 = f1_1;

    return 0;
}
