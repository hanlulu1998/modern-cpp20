// 4. __has_include判断头文件是否可以被包含进来
#if __has_include(<iostream>)
#include <iostream>
#endif
#include <cstdio>

#define LOG(msg, ...) printf("[" __FILE__ ":%d] " msg "\n", __LINE__, __VA_ARGS__)
// 如果...不存在，那么,编译就会错误，clang和gcc可以使用##
#define LOG1(msg, ...) printf("[" __FILE__ ":%d] " msg "\n", __LINE__, ##__VA_ARGS__)
// 现在VA_OPT宏可以实现上述功能
#define LOG2(msg, ...) printf("[" __FILE__ ":%d] " msg "\n", __LINE__ __VA_OPT__(, ) __VA_ARGS__)

// 1. 编译器属性语法
// 1）gcc/clang使用__attribute__((attribute-list))
// 2）mscv使用__declspec(attributelist)

// 2. c++标准：
// [[attr]]
// [[attr1, attr2, attr3(args)]]
// [[namespace::attr(args)]]
// [[ using attribute-namespace : attribute-list ]]
// 编译器会自动忽略不识别的属性
[[using gnu: always_inline, hot, const]] [[nodiscard]]
inline int f() {
    return 0;
}

[[deprecated]] void foo() {
}
class [[deprecated]] X {};

void bar() {
}
void foo(int a) {
    switch (a) {
    case 0:
        break;
    case 1:
        bar();
        [[fallthrough]];
    case 2:
        bar();
        break;
    default:
        break;
    }
}

[[nodiscard("Memory leak!")]] char *char_malloc() {
    return new char[100];
};

class T {
  public:
    [[nodiscard]] T() {
    }
    explicit T(int) {
    }
};

int zoo(int a [[maybe_unused]], int b [[maybe_unused]]) {
    return 10;
}

int f(int x) {
    if (x == 0) [[unlikely]] // 很少为 true
        return 0;

    if (x > 0) [[likely]] // 大部分时候为 true
        return x;

    return -x;
}

struct Empty {};
struct Empty1 {};
struct A {
    int i;
    [[no_unique_address]] Empty e, e1;
    [[no_unique_address]] Empty1 e2;
};

int main(void) {
    // 3. 9个标准属性
    // 1) noreturn：声明函数不会返回，和void不同，汇编直接会取消函数调用和栈操作
    // 2）carries_dependency：允许跨函数传递内存依赖项，它通常用于弱内存顺序架构平台上多线程程序的优化，避免编译器生成不必要的内存栅栏指令
    // 3）deprecated带有此属性的实体被声明为弃用
    X x;
    foo();
    // 4）fallthrough：在switch语句的上下文中提示编译器直落行为是有意的，并不需要给出警告
    foo(1);
    // 5) nodiscard：声明函数的返回值不应该被舍弃，否则鼓励编译器给出警告提示
    // nodiscard属性也可以声明在类或者枚举类型上，但是它对类或者枚举类型本身并不起作用，只有当被声明为nodiscard属性的类或者枚举类型被当作函数返回值的时候才发挥作用
    // nodiscard属性支持将一个字符串字面量作为属性的参数，该字符串会包含在警告中，可以用于解释返回结果不应被忽略的理由
    // nodiscard属性还能用于构造函数，它会在类型构建临时对象的时候让编译器发出警告
    char_malloc();
    T{};
    // 6) maybe_unused：声明实体可能不会被应用以消除编译器警告
    // maybe_unused属性除作为函数形参属性外，还可以用在很多地方，比如类、结构体、联合类型、枚举类型、函数、变量
    zoo(1, 2);
    // 7）likely和unlikely：声明在标签或者语句上，通常在switch和if上，
    // likely属性允许编译器对该属性所在的执行路径相对于其他执行路径进行优化；而unlikely属性恰恰相反
    // likely告诉编译器这条路径大概率执行，unlikely表示很少执行
    f(2);
    // 8) no_unique_address：指示编译器该数据成员不需要唯一的地址，也就是说它不需要与其他非静态数据成员使用不同的地址
    // 注意，该属性声明的对象必须是非静态数据成员且不为位域

    // 存在两个相同的类型且它们都具有no_unique_address属性，那么编译器不会重复地将其堆在同一地址
    // 如果是两个不同类型则可以
    std::cout << "sizeof(X) = " << sizeof(A) << std::endl
              << "A::i address = " << &((A *)0)->i << std::endl
              << "A::e address = " << &((A *)0)->e << std::endl
              << "X::e1 address = " << &((A *)0)->e1 << std::endl
              << "X::e2 address = " << &((A *)0)->e2 << std::endl;
    // 5. 新的VA_OPT宏
    LOG("Hello %d", 2025);
    LOG1("Hello 2025");
    // VA_OPT通过可变参数数目来选择这个逗号是否存在
    LOG2("Hello 2025");

    return 0;
}
