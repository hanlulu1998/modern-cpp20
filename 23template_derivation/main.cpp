#include <cstddef>
#include <iostream>
#include <tuple>
#include <vector>

template <class T>
struct LambdaWarp {
    LambdaWarp(T t) : func(t) {
    }

    template <class... Args>
    void operator()(Args &&...arg) {
        func(std::forward<Args>(arg)...);
    }
    T func;
};

template <class T, class U>
struct C {
    C(T, U) {
    }
};

template <class V>
using A = C<V *, V *>;

template <class T>
struct S {
    T x;
    T y;
};

template <class T, class U>
struct X {
    S<T> s;
    U u;
    T t;
};

template <class T, size_t N = 5>
struct Num {
    T array[N];
};

// 6. 用户自定义推导
template <typename T1, typename T2>
struct MyPair {
    MyPair(const T1 &x, const T2 &y)
        : first(x), second(y) {
    }
    T1 first;
    T2 second;
};
template <typename T1, typename T2>
inline MyPair<T1, T2>
make_mypair(T1 x, T2 y) {
    return MyPair<T1, T2>(x, y);
}

// 定义我们的推导，让const char(&)[N] 为const char*
template <typename T, size_t N>
MyPair(T, const char (&)[N]) -> MyPair<T, const char *>;

// 7. SFINAE（替换失败而不是错误）
// 函数模板重载时，当模板形参替换为指定的实参或由函数实参推导出模板形参的过程中出现了失败，则放弃这个重载而不是抛出一个编译失败
// 标准中规定，在直接上下文中使用模板实参替换形参后，类型或者表达式不符合语法，那么替换失败；而替换后在非直接上下文中产生的副作用导致的错误则被当作编译错误
// 也就是说如果替换失败后，编译器确定了结果，最后运行在上下文还是失败了，那么就是编译错误
template <class T>
T foo(T &t) {
    T tt(t);
    return tt;
}

void foo(...) {
    std::cout << "foo: SFINA\n";
}

class SomeObj1 {
  public:
    void Dump2File() const {
        std::cout << "dump this object to file" << std::endl;
    }
};

class SomeObj2 {
};

template <class T>
auto DumpObj(const T &t) -> decltype(((void)t.Dump2File()), void()) {
    t.Dump2File();
}

void DumpObj(...) {
    std::cout << "the object must have a member function Dump2File" << std::endl;
}

int main(void) {
    // 1. 类模板参数现在也和函数模板参数一样可以自动推导
    auto t1 = std::make_tuple(5, 11.7, "hello world");
    std::tuple t2{5, 11.7, "hello world"};

    // 2. 拷贝初始化优先
    std::vector v1{1, 3, 5};
    // 这里的v2类型不是std::vector<std::vecotr<int>>
    // 默认采用拷贝优先，所以实际上v2和v1的类型一致
    std::vector v2{v1};

    // 3. lambda自动推导，不用再使用decltype()声明lambda类型了
    LambdaWarp l([](int a, int b) {
        std::cout << a + b << std::endl;
    });
    l(11, 7);

    // 4. 别名自动推导
    int i{};
    A a1(&i, &i);

    // 5. 聚合类型推导
    // 这里的S<T>不参与推导，T=int是4推导的
    X x{{1, 2}, 3u, 4};

    Num n{{1, 2, 3}};

    MyPair<int, const char *> p1(5, "hello world");
    // 这里会失败，因为hello wrold是const char[12]，我们的引用没有触发衰退
    // 只要是按值传递的函数参数，数组一定衰退为指针，否则不会衰退
    // 在定义用户推导后就可以正确使用了
    MyPair p2{5, "hello world"};
    // 我们使用传值的方式可以实现衰退
    auto p3 = make_mypair(5, "hello world");

    double f = 7.0;
    int re = foo(f);
    std::cout << re << "\n";
    // 由于无法推导产生替换失败后，只能切换到foo(...)
    foo(5);

    // 这里我们使用了decltype(((void)t.Dump2File()), void())
    // 如果函数成员存在符合语法，我们就重载，否则触发替换失败，放弃重载
    DumpObj(SomeObj1());
    DumpObj(SomeObj2());

    return 0;
}
