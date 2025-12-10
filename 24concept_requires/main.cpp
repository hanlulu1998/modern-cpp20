#include <array>
#include <cmath>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
// 1. 传统使用std::enable_if约束模板
// 对于std::enable_if，如果模板实参为true则有type值，否则没有，不合语法，触发SFINAE
template <class T, class U = std::enable_if_t<std::is_integral_v<T>>>
struct X {};

// 2. 使用概念
// 合取
template <class C>
concept SignedIntType = std::is_integral_v<C> && std::is_signed_v<C>;
// 析取
template <class C>
concept IntFloatType = std::is_integral_v<C> || std::is_floating_point_v<C>;

template <SignedIntType T>
struct X1 {};

template <IntFloatType T>
struct X2 {};

// 3. 使用约束requires
constexpr bool bar() {
    return true;
}

// 为了防止歧义，requires要求是初等表达式或者带()的任意表达式
// 常量函数不是初等表达式，需要带括号
template <class T>
    requires(bar()) && std::is_integral_v<T>
struct X3 {};

// requires除了可以放在函数前面，还可以放在函数后面
template <class T>
    requires std::is_integral_v<T>
void foo1();

template <class T>
void foo2()
    requires std::is_integral_v<T>;

// 4. concept与requires的优先级
// 编译器规定按照出现顺序约束
template <class C>
concept ConstType = std::is_const_v<C>;

template <class C>
concept IntegralType = std::is_integral_v<C>;

template <ConstType T>
    requires std::is_pointer_v<T>
void foo(IntegralType auto)
    requires std::is_same_v<T, char *const>
{
}

// 5. 原子约束
// 1）原子约束不会简化代数表达式，而是直接匹配AST
template <int N>
concept Add1 = (N == 1);

template <int N>
void f2()
    requires Add1<2 * N + 1>
{
}

// 这里的2*N+1和N*2+1不相同，所以编译器不会认为等价
// 所以两个条件都符合会出现二义性，不知道匹配那个
// template <int N>
// void f2()
//     requires(Add1<N * 2 + 1>)
// {
// }
// 2）原子约束也不会化简逻辑，逻辑和算术相等，不等于约束相等，约束需要看的是表达式结构
// 当约束表达式中存在原子约束时，如果约束表达式结果相同，则约束表达式应该是相同的，否则会导致编译失败

// 这里的原子约束为sad，但是约束表达式结构不同，所以出现二义性
template <class T>
concept sad = false;
template <class T>
int fn1(T)
    requires(!sad<T>)
{
    return 1;
};
template <class T>
int fn1(T)
    requires(!sad<T>) && true
{
    return 2;
};

// 原子约束为no_sad，表达式原子约束仅来自concept-id
// 3) 标准规定：允许将与true的conjunction视为不添加额外约束，在匹配时优先选择复杂的那个
template <class T>
concept not_sad = !sad<T>;
template <class T>
int fn2(T)
    requires(not_sad<T>)
{
    return 3;
};
template <class T>
int fn2(T)
    requires(not_sad<T>) && true
{
    return 4;
};

// 6. requeires表达式
// 对requires表达式进行模板实参的替换
// 如果替换之后requires表达式中出现无效类型或者表达式违反约束条件，则requires表达式求值为false
// 反之则requires表达式求值为true
template <class T>
// 对于类型的操作，我们一般使用带参数的requires表达式
// concept Check = requires {
//     T().clear();
//     T() + T();
// };

// 这里的参数没有存储和生命周期一说，所以不定参数列表不支持
concept Check = requires(T a, T b) {
    a.clear();
    a + b;
};
template <Check T>
struct G {};

// 1) 简单要求：不以requires关键字开始的要求，它只断言表达式的有效性，并不做表达式的求值操作，如上述的a+b
// 2）类型要求：以typename关键字开始的要求，紧跟typename的是一个类型名，通常可以用来检查嵌套类型、类模板以及别名模板特化的有效性
template <typename T, typename T::type = 0>
struct D;
template <typename T>
using Ref = T &;
template <typename T>
concept C = requires {
    typename T::inner; // 要求嵌套类型
    typename D<T>;     // 要求类模板特化
    typename Ref<T>;   // 要求别名模板特化
};

template <C c>
struct M {};

struct H {
    using type = int;
    using inner = double;
};
// 3) 复合要求
// {E} noexcept->F，这里的noexcept和F可选
// 首先E要有效，其次noexcept不能抛出异常，最后F的类型要正确
template <class T>
concept Check1 = requires(T a, T b) {
    { a.clear() } noexcept;
    { a + b } noexcept -> std::same_as<int>;
};
// 嵌套要求，通常根据局部形参来指定其他额外的要求
template <class T>
concept Check2 = requires(T a, T b) {
    requires std::same_as<decltype((a + b)), int>;
};

// 7. 约束可变参数模板
// s1包展开后为C1<T1> && C1<T2> && ...
template <class T>
concept C1 = true;
template <C1... T>
struct s1 {};

// s2包展开后还是C2<T1> && C2<T2> && ...
template <class... Ts>
concept C2 = true;
template <C2... T>
struct s2 {};

// s3包展开后是C3<T1,int> && C3<T2,int> && ...
template <class T, class U>
concept C3 = true;
template <C3<int>... T>
struct s3 {};

// 8. 约束特化模板
template <typename T>
concept R = requires(T t) { t.f(); };
template <typename T>
struct S {
    S() {
        std::cout << "1.template<typename T> struct S" << std::endl;
    }
};
template <R T>
struct S<T> {
    S() {
        std::cout << "2.template<C T> struct S<T>" << std::endl;
    }
};
struct Arg {
    void f();
};

// 9. 约束auto和decltype(auto)
IntegralType auto bar1() {
    return 1;
}

int main(void) {

    X<int> x;

    X1<int> x1;
    X2<double> x2;
    X3<int> x3;

    foo<char *const>(1);

    f2<0>();

    std::cout << fn2(0) << "\n";

    // std::vector没有+操作符，失败
    // G<std::vector<char>> g1;
    G<std::string> g2;
    // std::array没有clear()，失败
    // G<std::array<char, 10>> g3;

    // 这里会要求检查T::inner、S<T>和Ref<T>
    // 如果替换失败就会返回false
    M<H> m;

    // 由于Arg满足有函数f()，所以特化优先选择
    S<int> s1;
    S<Arg> s2;

    // 在任何地方都可以约束
    // 约束的auto或decltype(auto)总是紧随约束之后，cv符号不能随便移动
    IntegralType auto i = 11;
    IntegralType decltype(auto) i2 = 11;

    auto bar2 = []() -> IntegralType auto { return 11; };
    const IntegralType auto i5 = 23;
    IntegralType auto const i6 = 8;
    // IntegralType const auto i7 = 6;

    return 0;
}
