#include <initializer_list>
#include <iostream>
#include <map>
#include <string>
#include <vector>
// 1. 非静态默认初始化
// 1)不得使用auto
// 2）不得使用()初始化
// 3)默认初始化优先级最低
struct A {
    int a = 1;
    float b{1.0};
    static inline int c{2};
    static const int d = 2;
};

// 2. 位域默认初始化
struct S {
    int x : 8 = 11;
    int z : 4 {7};
};

// 3. 位域宽度必须是常量表达式
// 1）c++20允许 true ? 常量表达式 : 非常量表达式
// 只要条件常量，未选中的分支就不要求常量表达式
// 2）注意这里的位域宽度表达式必须使用括号，否则后续的表达式都作为位域宽度
int a;
struct S2 {
    int y : (true ? 8 : a) = 42;
    int z : (1 || new int){0};
};
// 7. 想使用初始化列表，参数使用std::initializer_list即可
struct C {
    C(std::string a, int b) {
    }
    C(int a) {
    }
    C(std::initializer_list<std::string> a) {
    }
};

void foo(C) {
}

struct Point {
    int x = 0;
    int y = 0;
};

struct Point2 {
    Point a{0, 0};
    Point b{0, 0};
};

union U {
    int a;
    const char *b;
};

int main(void) {
    // 4. 初始化列表
    // 1）默认使用
    C x = {5};         // 拷贝初始化
    C x1{5};           // 直接初始化
    foo({2});          // 拷贝初始化
    foo({"hello", 8}); // 拷贝初始化
    // 2)支持STL
    std::vector<int> v{1, 2, 3, 4, 5};
    // 注意这里的{}和()是不一样的，()是构造
    std::vector<int> v1(10, 0);
    std::map<int, int> m{{1, 2}, {2, 3}};

    // 5.初始化列表的窄收缩，值不匹配不通过
    float f{1.0};
    long long ll{100ll};

    // 6.指定初始化
    // 1)可以省略指定也可以全部指定，但不能像C一样乱序
    Point p{.x = 1, .y = 2};
    Point p1{.y = 3};
    // 2)不可使用嵌套初始化
    // .a.x = 2只能在c中使用
    Point2 p2{.a{.x = 2}};
    // 3) 联合体数据成员只能初始化一个
    U u = {.a = 1};
    U u1 = {.b = "hello world"};
    // 4) c语言数组指定初始化不支持，因为和lambda冲突
    // int a[5] = {[1]=0};
    int a[5]{0};

    // 7. 自定义使用初始化列表
    C c{"hello","world","!!!"};
    
    return 0;
}
