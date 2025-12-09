#include "header.h"
#include <iostream>
#include <sys/types.h>
#include <vector>

// 1. 模板别名
// 使用using创建模板的一个别名
template <typename T>
using Vec = std::vector<T>;

// 2. typename优化
// 1）允许使用typename声明模板的模板形参
template <typename T>
struct A {
    static void print_type() {
        std::cout << typeid(T).name() << "\n";
    }
};
// 此前只能使用template<typename> class T
// 现在由于模板别名的原因放开了
template <template <typename> typename T, typename U>
struct B {
    using type = T<U>;
};
// 2）减少不必要的typename使用
// 当使用未决类型的内嵌类型时，例如X<T>::Y，需要使用typename明确告知编译器X<T>::Y是一个类型，除了指定基类和成员初始化外
// 现在有了更多可以省略的时候：
// a） static_cast、const_cast、reinterpret_cast或dynamic_cast等类型转换
// b）定义类型别名using R=T::B
// c）后置返回类型
// d）模板默认参数
// e）全局或者命名空间中简单的声明或者函数的定义：template<class T> T::R f();
// f）结构体成员
// g）成员函数或者lambda表达式形参声明

// 3. 外部模板
// 默认模板的在每个TU生成后，编译器会去重只保留一份
// 使用extern关键字可以让TU单元的模板不自己生成，而是查找外部相同生成，减少生成和去重的消耗
extern template class Bar<int>;

// 4. 连续的尖括号解析优化
// 很久之前默认连续右尖括号会识别为>>，必须使用空格
using IntTable = std::vector<std::vector<int>>;
// 现在已经不需要了
using BoolTable = std::vector<std::vector<bool>>;

// 5. friend声明友元类型时可以不用class关键字
class C;
typedef C Ct;

class X {
    friend C;
    friend Ct;
    friend void;
    friend int;
};
// 当友元是基本类型时，编译会忽略
// 这里就可以利用这一点，通过类型来限制友元接口的可见性
template <typename T>
class R {
    friend T;
};

// 6. 变量模板
// 使用类模板定义静态常量成员
template <class T>
struct PI1 {
    static constexpr T value = static_cast<T>(3.1415926);
};
// 使用常量函数
template <class T>
constexpr T PI2() {
    return static_cast<T>(3.1415926);
}

// 现在我们可以使用变量模板，这里并不限制一定使用常量
template <class T>
constexpr T PI = static_cast<T>(3.1415926);

// 7. 一些重要优化
// 1）允许常量求值作为非类型模板的实参
// NTTP要求的实参必须是静态存储或者可链接全局的值，也就是编译期间能够确定地址和值
// 但之前要求只能是整型常量的转换可以作为实参，其他不行
// 现在已经放开了，非类型模板形参使用的实参可以是该模板形参类型的任何经转换常量表达式。其中经转换常量表达式的定义添加了对象、数组、函数等到指针的转换

template <const int *>
void fun() {
}

int gvalue = 12;
constexpr int *get_gvalue_addr() {
    return &gvalue;
}

template <const char *>
struct Y {};

// 2）允许匿名和局部类型作为模板参数
// 3) 函数模板现在也支持默认模板参数了，依旧是从右向左定义


int main(void) {
    B<A, int>::type::print_type();
    Vec<int> v{1, 2, 3, 4};

    R<C> rc;
    R<Ct> rct;
    R<int> ri;
    R<void> rv;

    std::cout << PI1<float>::value << "\n";
    std::cout << PI2<float>() << "\n";
    std::cout << PI<float> << "\n";

    fun<&gvalue>();

    static int a = 10;
    fun<&a>();

    int b = 2;
    // 这里的b不行，不具有上述性质
    // fun<&b>();

    // 现在这个已经可以编译成功了
    fun<get_gvalue_addr()>();

    // 支持const char*的常量表达
    static const char str[] = "hello";
    Y<str> y;

    return 0;
}
