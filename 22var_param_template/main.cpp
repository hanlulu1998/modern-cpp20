#include <cstddef>
#include <functional>
#include <iostream>
#include <tuple>
#include <utility>

// 1. 可变参数模板
// class ...Args叫类型模板形参包
// Args ...args叫函数参数形参包
// 1）函数模板，模板参数包可以出现在任何位置
template <class... Args, class T>
void foo(T, Args... args) {
}
// 2）类模板，模板参数包必须是最后一个
template <class T, class... Args>
class Bar {};

// 2. 形参包展开

// 1）基本展开
template <class... T>
int baz(T... t) {
    return 0;
}

template <class... Args>
void zoo(Args... args) {
}

template <class... Args>
class Var {
  public:
    Var(Args... args) {
        // 第一步对baz中的&args...展开，&args是模式，所以变成baz(&args1,&args2,...)
        // 第二步对baz(&args...) + args ...展开，baz(&args...) +args是模式，所以变成zoo(baz(&args1,&args2,...)+args1,baz(&args1,&args2,...)+args2,...)
        zoo(baz(&args...) + args...);
    }
};
// 2）函数展开
int add(const int a, const int b) {
    return a + b;
}

inline int sub(const int a, const int b) {
    return a - b;
}

// 使用Args (*...args)(const int, const int)函数参数包
template <class T, class U, class... Args>
void for_each_call(T t, U u, Args (*...args)(const int, const int)) {
    // 这里使用逗号表达式作为模式，让每一个参数都能展开调用
    int temp[]{(std::cout << args(t, u) << std::endl, 0)...};
}

// 3）继承展开
template <class... Args>
class Derived : public Args... {
  public:
    Derived(const Args &...args) : Args(args)... {
    }
};

class Base1 {
  public:
    Base1() = default;
    Base1(const Base1 &) {
        std::cout << "Base1 cotr\n";
    }
};

class Base2 {
  public:
    Base2() = default;
    Base2(const Base2 &) {
        std::cout << "Base2 cotr\n";
    }
};

// 4) 模板形参包
template <template <typename...> typename... Args>
class Te : public Args<int, double>... {
  public:
    Te(const Args<int, double> &...args) : Args<int, double>(args)... {
    }
};

template <class... Args>
class Xe1 {};

template <class... Args>
class Xe2 {};

// 两个形参包混合，只要解包长度相同就可以联立解包
template <class...>
struct Tuple {};
template <class T1, class T2>
struct Pair {};
template <class... Args1>
struct zip {
    template <class... Args2>
    struct with {
        typedef Tuple<Pair<Args1, Args2>...> type;
    };
};

// 5) lambda捕获
template <class... Args>
class LM {
  public:
    LM(Args... args) {
        auto lm = [args...] {
            zoo(&args...);
        };
        lm();
    }
};

// 3. sizeof...运算符号
// 可以对函数形参包和类型模板参数包使用，返回大小
template <class... Args>
struct S {
    static constexpr size_t get_size() {
        return sizeof...(Args);
    }
};

template <class... Args>
constexpr size_t get_size(Args... args) {
    return sizeof...(args);
}

// 4. 递归计算包展开
template <typename T>
T sum(T arg) {
    return arg;
}

template <typename T, typename... Args>
auto sum(T arg1, Args... args) {
    return arg1 + sum(args...);
}

// 5. 折叠表达式
template <class... Args>
auto sum2(Args... args) {
    return (args + ...);
}
// 1) 一元向右折叠
// (args op ...) = args1 op (args2 op (args3 op ...   ))
// 2）一元向左折叠
// (... op args) = (((args1 op arg2) op args3) op args4) op ...
// 3) 二元向右折叠
// (args op ... op init) = args1 op (args2 op (args3 op ....(argN op init)))
// 4）二元向左折叠
// (init op ... op args) = (((args1 op init) op args2) op args3) op ...

template <class... Args>
void print(Args... args) {
    (std::cout << ... << args) << std::endl;
}

// 6. 一元折叠空参数包问题
// 1) 只有&&、||和,运算符能够在空参数包的一元折叠表达式中使用
// 2) &&的求值结果一定为true
// 3) ||的求值结果一定为false
// 4) ,的求值结果为void()
template <typename... Args>
auto andop(Args... args) {
    return (args && ...);
}

// 7. using声明中的包展开
template <class T>
class M {
  public:
    M() = default;
    M(T t) : t_(std::move(t)) {
    }

  private:
    T t_;
};

template <class... Args>
class N : public M<Args>... {
  public:
    using M<Args>::M...;
};

// 8. lambda初始化捕获的包展开
// 之前想要使用move的方式
template <class F, class... Args>
auto delay_invoke(F f, Args... args) {
    return [f = std::move(f), tup = std::make_tuple(std::move(args)...)]()
               -> decltype(auto) {
        return std::apply(f, tup);
    };
}
// 现在的方式
template <class F, class... Args>
auto delay_invoke2(F f, Args... args) {
    // 注意这里的参数是...args
    return [f = std::move(f), ... args = std::move(args)]() -> decltype(auto) {
        return std::invoke(f, args...);
    };
}

int main() {
    unsigned int x = 8;
    foo(1);          // foo<int>(int)
    foo(1, 11.7);    // foo<int,double>(int,double)
    foo(1, 11.7, x); // foo<int,double,unsigned int>(int,double,unsigned int)

    Bar<int, double> b;
    for_each_call(12.0, 10.0, sub, add);

    Base1 base1;
    Base2 base2;

    Derived<Base1, Base2> d(base1, base2);

    Xe1<int, double> xe1;
    Xe2<int, double> xe2;
    Te<Xe1, Xe2> te(xe1, xe2);

    zip<short, int>::with<unsigned short, unsigned>::type t1;

    LM<int, double> lm(1, 2.0);

    std::cout << S<int, double>::get_size() << "\n";
    std::cout << S<>::get_size() << "\n";

    std::cout << get_size(1, 1.1, true) << "\n";
    std::cout << get_size() << "\n";

    std::cout << sum(1, 2, 3, 4, 5) << "\n";
    std::cout << sum2(1, 2, 3, 4, 5) << "\n";

    print("hello", "c++", 20);

    std::cout << std::boolalpha << andop() << "\n";

    N<int, std::string, bool> n1{11};
    N<int, std::string, bool> n2{std::string("Hello World")};
    N<int, std::string, bool> n3{true};
}
