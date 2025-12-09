#include <iostream>
#include <string>
#include <type_traits>
// 1. constexpr关键字
// 1）用法与const基本一致，表达常量含义，会使变量具有const属性
const int a = 10;
constexpr int b = 10;

constexpr int const_abs(int x) {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}

struct T {
    bool val;
};

constexpr int sum(int x) {
    int result = 0;
    // 平凡构造的类型可以默认初始化，而不进行手动初始化
    T val;
    while (x > 0) {
        result += x--;
    }
    return result;
}

class X {
  public:
    constexpr X() : x1(5) {
    }
    constexpr X(int i) : x1(0) {
        if (i > 0) {
            x1 = 5;
        } else {
            x1 = 8;
        }
    }
    constexpr void set(int i) {
        x1 = i;
    }
    // 现在constexpr成语函数不再具有const属性，需要手动添加
    constexpr int get() const {
        return x1;
    }

    constexpr ~X() {
    }

    // c++17之后static constexpr默认具有inline属性
    static constexpr int num{5};

  private:
    int x1;
};

constexpr X make_x() {
    X x;
    x.set(42);
    return x;
}

template <class T>
auto any2i(T t) {
    // 使用嵌套保证短路操作
    if constexpr (std::is_same<T, std::string>::value) {
        if constexpr (T::npos == -1) {
            return atoi(t.c_str());
        }
    } else {
        return t;
    }
}

struct X1 {
    virtual int f() const = 0;
};

struct X2 : public X1 {
    constexpr virtual int f() const {
        return 2;
    }
};

struct X3 : public X2 {
    virtual int f() const {
        return 3;
    }
};

struct X4 : public X3 {
    constexpr virtual int f() const {
        return 4;
    }
};

consteval int seq(const int x) {
    return x * x;
}

// 7. 不管编译顺序如何，都能保证依赖config_value的TU获得正确的值
constinit int config_value = 12;

int main(void) {
    int arr1[a]{0};
    int arr2[b]{0};

    const int *a_ptr = &a;
    const int *b_ptr = &b;
    // 编译器默认直接替换，仅当取地址时初始化内存值
    std::cout << a_ptr << ": " << *a_ptr << "\n";
    std::cout << b_ptr << ": " << *b_ptr << "\n";

    // 2) constexpr必须保持编译期间的常量性
    int x1 = 1;
    const int x2 = x1;
    // 这一句报错，因为常量表达式必须由常量初始化
    // constexpr int x3 = x1;

    // 2. 常量表达式函数
    // 1) 允许声明变量，但是没有初始化的变量，static和thread_local不行，这里如果是平凡的构造是可以的
    // 2）可以使用if和switch以及任何循环，但是不能用goto
    // 3）函数可以修改生命周期和常量表达式相同的对象，包含联合体的成员
    // 4）函数返回值可以是void
    // 5）constexpr的成员函数不再具有默认的const属性（c++14之前是默认具有的）
    // 6）函数可以try-catch但不能throw，不过对于常量函数这个没什么意义
    char buff[sum(5)]{0};
    char buff1[const_abs(-5)]{0};

    // 6) 常量表达式函数可以退化成运行时函数
    int x3 = 100;
    int x4 = const_abs(100);
    std::cout << x4 << "\n";

    // 3. constexpr构造函数
    // 1）constexpr自定义类型，用户自定义的析构必须是constexpr的
    {
        constexpr X x1(-1);
        constexpr X x2 = make_x();
        constexpr int a1 = x1.get();
        constexpr int a2 = x2.get();
        std::cout << a1 << std::endl;
        std::cout << a2 << std::endl;
    }

    // lambda函数默认是constexpr的
    auto get_size = [](int i) { return i * 2; };
    {
        char buff[get_size(5)]{0};
    }

    // 1) 当不满足constexpr时自动退化为运行时函数
    auto get_count = []() {
        static int x = 5;
        return x;
    };
    std::cout << get_count() << "\n";
    // 2) 使用constexpr后缀强制使用常量表达
    // auto get_count2 = []()constexpr {
    //     static int x = 5;
    //     return x;
    // };
    //

    // 4. if constexpr
    // 1）if constexpr的条件必须是编译期能确定结果的常量表达式
    // 2）条件结果一旦确定，编译器将只编译符合条件的代码块，类似于宏定义
    // 3）if constexpr不支持短路规则，也就是所有所有条件都会运行，如果要实现短路可以使用嵌套
    std::cout << any2i("01") << "\n";
    int num = any2i(std::string("12"));
    std::cout << num << "\n";

    // 5. contexpr虚函数

    {
        constexpr int (X1::*pf)() const = &X1::f;

        constexpr X2 x2;
        static_assert(x2.f() == 2);
        static_assert((x2.*pf)() == 2);

        // 注意constexpr会让变量带上const属性，所以我们的&和*必须要是const&和const*
        // 截止到2025 Clang依旧不支持constexpr虚函数

        // constexpr X1 const &r2 = x2;
        // static_assert(r2.f() == 2);
        // static_assert((r2.*pf)() == 2);

        // constexpr X1 const *p2 = &x2;
        // static_assert(p2->f() == 2);
        // static_assert((p2->*pf)() == 2);

        // constexpr X4 x4;
        // static_assert(x4.f() == 4);
        // static_assert((x4.*pf)() == 4);

        // constexpr X1 const &r4 = x4;
        // static_assert(r4.f() == 4);
        // static_assert((r4.*pf)() == 4);

        // constexpr X1 const *p4 = &x4;
        // static_assert(p4->f() == 4);
        // static_assert((p4->*pf)() == 4);
    }

    // 6. consteval立即函数
    // 修饰函数时要求函数必须是常量表达式，与constexpr区别是不可退化
    {
        int a = seq(4);
        // int b = seq(a);
    }

    // 7. costinit变量必须在编译期初始化，但不要求变量本身是 const
    // 只对全局/静态/线程存储期变量有效
    // 用来避免静态初始化顺序问题

    // 8. 判断常量求值环境std::is_constant_evaluated

    {
        // 1）根据上下文判断，是否是const环境
        auto foo = []() constexpr {
            const int n = 13;
            int m = std::is_constant_evaluated() ? 13 : 17;

            char arr[n]{};
            return m + sizeof(arr);
        };
        {
            const int p = foo();
            std::cout << p << "\n";
            const int q = p + foo();
            std::cout << q << "\n";
        }
        {
            const int p = foo();
            std::cout << p << "\n";
            int q = p + foo();
            std::cout << q << "\n";
        }
        {
            int p = foo();
            std::cout << p << "\n";
            int q = p + foo();
            std::cout << q << "\n";
        }

        // 2）当判断是否为明显常量求值时存在多个条件，会尝试各种可能
        int y = 0;
        // 评估为true时失败，使用false初始化为1
        const int a = std::is_constant_evaluated() ? y : 1;
        std::cout << a << "\n";
        // 评估为true直接初始化为2
        const int b = std::is_constant_evaluated() ? 2 : y;
        std::cout << b << "\n";
    }

    return 0;
}
