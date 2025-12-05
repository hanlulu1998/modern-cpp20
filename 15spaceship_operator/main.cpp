#include <compare>
#include <iostream>
#include <string>

struct B {
    int a;
    long b;
    // 默认情况下不会生成默认的三向比较符，需要用户显式默认声明
    auto operator<=>(const B &) const = default;
};

struct D : B {
    short c;
    auto operator<=>(const D &) const = default;
};

int ci_compare(const char *s1, const char *s2) {
    while (tolower(*s1) == tolower(*s2++)) {
        if (*s1++ == '\0') {
            return 0;
        }
    }
    return tolower(*s1) - tolower(*--s2);
}

// 定义一个大小写不敏感的字符串
class CIString {
  public:
    CIString(const char *s) : str_(s) {
    }

    std::weak_ordering operator<=>(const CIString &b) const {
        return ci_compare(str_.c_str(), b.str_.c_str()) <=> 0;
    }
    // 这里必须手动实现==
    bool operator==(const CIString &b) const {
        return ci_compare(str_.c_str(), b.str_.c_str()) == 0;
    }

  private:
    std::string str_;
};

struct E : B {
    CIString c{""};
    auto operator<=>(const E &) const = default;
};

struct F : E {
    float f = 1.0;
    auto operator<=>(const F &) const = default;
};

enum Color {
    RED
};

enum class StrongColor {
    RED,
    BLUE
};

using fp = void (*)();

void foo() {
}
void bar() {
}

struct Legacy {
    int n;
    bool operator==(const Legacy &rhs) const {
        return n == rhs.n;
    }
    bool operator<(const Legacy &rhs) const {
        return n < rhs.n;
    }
};

struct TreeWay {
    Legacy m;
    std::strong_ordering operator<=>(const TreeWay &) const = default;
};

int main(void) {
    // 1. 三向比较符又叫太空飞船符
    // lhs<=>rhs的表达式与0相比，等于0相当于lhs=rhs，小于0相当于lsh<rhs，大于0相当于lsh>rhs
    // 三向比较符的结果只能与0相比，因为其结果类型只实现了与nullptr_t类型的比较
    bool b = 7 <=> 11 < 0;
    std::cout << std::boolalpha << b << "\n";

    // 三向表达式按照结果返回三种类型：
    // 1）std::strong_ordering：若lhs == rhs，那么在任何情况下rhs和lhs都可以相互替换，也就是fx(lhs) == fx(rhs)
    // 三种结果：std::strong_ ordering::less、std::strong_ordering::equal以及std::strong_ ordering::greater
    D x1, x2;
    // 只要成员全是std::strong_ordering，那么复杂类型自己就是std::strong_ordering，这里整型、指针以及枚举默认是std::strong_ordering
    std::cout << typeid(decltype(x1 <=> x2)).name() << "\n";

    // 2) std::weak_ordering：若有lhs == rhs，则rhs和lhs不可以相互替换，也就是fx(lhs) != fx(rhs)
    // 这种类型在基础类型种不存在，但是用户可以自定义
    // 三种结果：std::weak_ ordering::less、std::weak_ordering::equivalent以及std::weak_ ordering::greater
    CIString s1{"HELLO"}, s2{"hello"};
    std::cout << (s1 <=> s2 == 0) << "\n"; // 输出为true
    std::cout << typeid(decltype(s1 <=> s2)).name() << "\n";

    // 当std::weak_ordering和std::strong_ordering同时存在，则复杂类型为std::weak_ordering
    E e1, e2;
    std::cout << typeid(decltype(e1 <=> e2)).name() << "\n";

    // 3）std::partial_ordering：比std::weak_ordering还要弱，当lhs == rhs时rhs和lhs不能相互替换，且多出一种没有关系的std::partial_ ordering::unordered结果
    // 四种结果：td::partial_ ordering::less、std::partial_ordering::equivalent、std::partial_ ordering::greater以及std::partial_ordering::unordered

    // 基础类型的浮点型默认就是这个，因为浮点存在NaN，与其他任何浮点没有比较关系
    std::cout << typeid(decltype(7.7 <=> 11.1)).name() << "\n";
    std::cout << ((0.0 / 0.0 <=> 1.0) == std::partial_ordering::unordered) << "\n";

    // 当std::weak_ordering和std::partial_ordering同时存在，则复杂类型为std::partial_ordering
    F f1, f2;
    std::cout << typeid(decltype(f1 <=> f2)).name() << "\n";

    // 2. 基础类型支持
    // 1）三向比较符同一般算数运算符一样，触发隐式转换
    std::cout << typeid(decltype(7 <=> 11.1)).name() << "\n";
    // 2) 枚举类型：无作用域枚举（传统枚举）会触发隐式整型转换，强枚举必须是相同的枚举类型
    std::cout << typeid(decltype(RED <=> 0)).name() << "\n";
    std::cout << typeid(decltype(StrongColor::BLUE <=> StrongColor::RED)).name() << "\n";
    // 3）bool类型要求两端都是bool类型
    std::cout << typeid(decltype(true <=> false)).name() << "\n";
    // 4）不支持数组名直接比较
    int arr1[5];
    int arr2[5];
    // std::cout << typeid(decltype(arr1 <=> arr2)).name() << "\n";
    // std::cout << typeid(decltype(foo <=> bar)).name() << "\n";
    // 5）比较符的一端为指针，另一个则会进行类型转换，比如将数组名转为指针，将派生类指针转为基类指针
    int *arr_ptr = arr2;
    std::cout << typeid(decltype(arr_ptr <=> arr1)).name() << "\n";

    // 6）函数指针不支持，需要手动实现
    fp foo_ptr = foo;
    fp bar_ptr = bar;
    // std::cout << typeid(decltype(foo_ptr <=> bar_ptr)).name() << "\n";

    // 3. 自动生成比较运算符
    // 1）旧的方式使用std::rel_ops命名空间后，只需要提供<和==，就可以实现其他四种运算法
    // 2）新的方式实现三向比较后，会自动生成<、<=、>和>=，但不会生成==和!=，用户还需要实现自己的==
    // 原因是性能问题，默认生成的==有巨大的性能问题，所以需要用户自己实现==，有了==后!=会自己生成
    bool r1 = s1 > s2;  // 调用三向比较符
    bool r2 = s1 == s2; // 调用==符
    // 3）兼容性：用户自定义类型中，实现了<、==运算符函数的数据成员类型，在该类型的三向比较中将自动生成合适的比较代码
    // 也就是三向比较符调用的还是数据成员自己定义的

    TreeWay t1, t2;
    bool r = t1 < t2;
    // 结构体TreeWay的三向比较操作会调用结构体Legacy中的<和==运算符来完成:
    // ```cpp
    // struct TreeWay {
    //   Legacy m;
    //   std::strong_ordering operator<=>(const TreeWay& rhs) const {
    //        if (m < rhs.m) return std::strong_ordering::less;
    //        if (m == rhs.m) return std::strong_ordering::equal;
    //        return std::strong_ordering::greater;
    //   }
    // };
    // ```
    return 0;
}
