#include <cstddef>
#include <iostream>
#include <new>

struct Boolean {
    int i;

    explicit operator bool() const {
        return i != 0;
    }
};

struct X {
    const int n;
};
union U {
    X x;
    float f;
};

struct T {
    T() {
    }
    T(const X &) = delete; // 即使拷贝被禁用也没问题
};

T f() {
    T x;
    return x;
}

struct C {
    int i;
    friend bool operator==(C, C) = default;
};

struct D {
    int i = 0;

    ~D() {
        std::cout << "dtor\n";
    }

    void *operator new(size_t size) {
        return ::operator new(size);
    }

    // 与传统delete函数相比就多了一个std::destroying_delete_t
    void operator delete(D *ptr, std::destroying_delete_t) {
        ptr->~D();
        std::cout << "call delete\n";
    }
};

// 9. 伪析构必定结束对象生命周期
// 过去伪析构如果是平凡类型会当成无效语句，但现在其伪析构一定结束对象生命周期
template <typename T>
void destroy(T *p) {
    p->~T();
}

int main(void) {
    // 1. 表达式求值顺序
    // 1) 函数表达式一定在函数参数之前
    // foo(a,b,c)也就是说foo一定在abc之前求值，但是abc顺序未知
    // 2) 后缀表达式、移位操作符和算数运算符，表达式求值总是从左往右
    // E1一定优先于E2
    // E1[E2]
    // E1.E2
    // E1.*E2
    // E1->*E2
    // E1<<E2
    // E1>>E2
    // E1 +-*/% E2
    // 3) E1@=E2这种形式，保证E2一定优先于E1
    // 4）new T(E) 保证new的内存分配一定优先于参数E的求值
    // 5）任何重载操作符函数，其求值顺序按照符号内置的顺序进行，而不是函数本身

    // 2. 显示自定义类型转换
    // 通过explicit可以禁止隐式的值转换，但是对于布尔转换，c++进行优化
    // 即使显性定义，在以下情况中依旧会隐式转换：
    // if、while、for的控制表达式
    // 内建逻辑运算符!、&&和||的操作数
    // 条件运算符?:的首个操作数
    // static_assert声明中的bool常量表达式
    // noexcept说明符中的表达式
    Boolean b{1};
    if (b) {
        std::cout << "true\n";
    }

    // 3. 在某个内存地址上重新构造了一个新对象（比如 placement new），那么指向原对象的指针不再合法，即使地址相同
    // std::launder的作用就是重新获取合法指向新对象的指针
    U u{{1}};
    X *x = new (&u.x) X{2};
    std::cout << x->n << "\n";
    // 按照标准来说u.x.n应该未定义
    // C++标准规定：如果新的对象在已被某个对象占用的内存上进行构建，那么原始对象的指针、引用以及对象名都会自动转向新的对象
    // 除非对象是一个常量类型或对象中有常量数据成员或者引用类型
    // 如果数据结构X的数据成员n不是一个常量类型，那么u.x.n的结果一定是2
    // 但是由于常量性的存在，从语法规则来说x已经不具备将原始对象的指针、引用以及对象名自动转向新对象的条件，因此结果是未定义的
    // 要访问n就必须通过新对象的指针x
    std::cout << u.x.n << "\n";
    // 使用std::launder可以让这个指针有效
    std::cout << *std::launder(&u.x.n) << "\n";

    // 4. 返回值优化与强制省略拷贝
    // 1）返回值优化分为RVO（Return Value Optimization）和NRVO（Named Return Value Optimization）
    // 当返回语句的操作数为临时对象时，为RVO；而当返回语句的操作数为具名对象时，为NRVO。二者效果一致
    // 2) c++20强制省略拷贝本质上就是要求编译器只要可以RVO就必须RVO即使拷贝函数不存在
    // 言保证对象直接在目标位置构造，不允许先构造临时再拷贝过去
    T t = f();

    // 5. 允许按值比较
    // 类的默认比较运算符函数可以是一个参数为const C&的非静态成员函数，或是两个参数为const C&或C的友元函数

    // 6. 支持new[] 自动推导长度
    int *arr = new int[]{1, 2, 3};
    int *arr1 = new int[5]{1, 2, 3};

    // 7. 允许数组转为未知范围数组
    auto unknown = [](int (&)[]) {};
    int arr2[1];
    unknown(arr2);
    auto unknow2 = [](int (*)[]) {};
    unknow2(&arr2);

    // 8. delete中自定义析构
    auto d = new D;
    delete d;

    // 10. 不推荐使用volatile
    // 1）不推荐算术类型的后缀++和--表达式以及前缀++和--表达式使用volatile限定符
    // 2）不推荐非类类型左操作数的赋值使用volatile限定符：E1 @= E2
    // 3）不推荐函数形参和返回类型使用volatile限定符
    // 4）不推荐结构化绑定使用volatile限定符
    // C++20标准的编译环境中编译都会给出警告

    // 11. 不推荐在下标中使用逗号表达式，这种表达将保留用在矩阵和向量中
    int a[]{1, 2, 3};
    std::cout << a[1, 2] << "\n";
    // 如果真的使用请使用()
    std::cout << a[(1, 2)] << "\n";


    return 0;
}
