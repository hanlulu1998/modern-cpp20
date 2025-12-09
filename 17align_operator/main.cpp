#include <cassert>
#include <cstddef>
#include <iostream>
#include <type_traits>
#include <memory>
struct A {
    char a1;
    int a2;
    double a3;
};

struct B {
    short b1;
    bool b2;
    double b3;
};

struct X {
    char a1;
    int a2;
    double a3;
};

struct X1 {
    alignas(16) char a1;
    alignas(double) int a2;
    double a3;
};

struct alignas(16) X2 {
    char a1;
    int a2;
    double a3;
};

struct alignas(16) X3 {
    alignas(8) char a1;
    alignas(double) int a2;
    double a3;
};

struct X4 {
    alignas(8) char a1;
    alignas(double) int a2;
    double a3;
};

#define COUT_ALIGN(s) std::cout << "alignof(" #s ") = " << alignof(s) << std::endl

int main() {
    // 1. 对齐问题，C/C++数据默认有成员对齐和整体对齐
    // 1） 成员对齐从a成员到b成员，如果a的内存大小小于b会进行填充后对齐到b的大小
    // 2）整体对齐内部成员最大对齐，其大小应为对齐的整倍数，不够则填充
    std::cout << "sizeof(A::a1) + sizeof(A::a2) + sizeof(A::a3) = "
              << sizeof(A::a1) + sizeof(A::a2) + sizeof(A::a3) << std::endl;
    std::cout << "sizeof(B::b1) + sizeof(B::b2) + sizeof(B::b3) = "
              << sizeof(B::b1) + sizeof(B::b2) + sizeof(B::b3) << std::endl;
    std::cout << "sizeof(A) = " << sizeof(A) << std::endl;
    std::cout << "sizeof(B) = " << sizeof(B) << std::endl;

    // 2. alignof获取对齐
    // c++同时提供std::alignment_of_v实现一样的功能
    int x1 = alignof(A);
    int x2 = alignof(B);
    std::cout << x1 << ", " << x2 << " == " << alignof(double) << "\n";
    // gcc/clang使用__aglinof__，msvc使用__alignof
    int x3 = __alignof__(double);
    std::cout << x3 << "\n";
    // alignof规定只能是类型，但是gcc扩展后可以是变量
    A a;
    int x4 = alignof(a);
    std::cout << x4 << "\n";

    // 使用alignof(decltype)本质上还是alignof(type)，结果还是默认值
    // 想要获得修改后的变量对齐，只能使用编译器内部的方式
    alignas(8) int b = 0;
    int x5 = alignof(decltype(b));
    std::cout << x5 << "\n";
    int x6 = __alignof__(b);
    std::cout << x6 << "\n";

    // std::max_align_t的值为平台标准类型最大的对齐长度
    std::cout << alignof(std::max_align_t) << "\n";
    // new和malloc之类的分配函数返回的指针需要适合于任何对象，也就是说内存地址至少与std::max_align_t严格对齐
    for (int i = 0; i < 100; i++) {
        auto *p = new char();
        // std::uintprt_t/std::intptr_t为依赖平台的指针类型，类似与size_t，保证能够容纳平台最大的数据长度
        // 可以做字节加法std::uintprt_t + n，原始指针不允许
        auto addr = reinterpret_cast<std::uintptr_t>(p);
        // 一定对齐std::max_align_t
        assert(addr % alignof(std::max_align_t) == 0);
        delete p;
    }

    // 3. alignas运算符，强制对齐
    // gcc和clang内置__attribute__((aligned(8)))
    // msvc内置__declspec(align(8))
    {
        X x;
        X1 x1;
        X2 x2;
        X3 x3;
        X4 x4;
        X3 x5;
        alignas(16) X4 x6;

        COUT_ALIGN(x);
        COUT_ALIGN(x1);
        COUT_ALIGN(x2);
        COUT_ALIGN(x3);
        COUT_ALIGN(x4);
        COUT_ALIGN(x5);
        COUT_ALIGN(x6);
        COUT_ALIGN(x5.a1);
        COUT_ALIGN(x6.a1);
    }

    // 4. 一些额外的对齐类型
    // 1) std::aligned_storage可以用来分配一块指定对齐字节长度和大小的内存
    std::aligned_storage_t<128, 16> buffer;
    std::cout << sizeof(buffer) << std::endl;
    std::cout << alignof(buffer) << std::endl;
    // 2) std::aligned_union接受一个std::size_t作为分配内存的大小，以及不定数量的类型
    // std::aligned_union会获取这些类型中对齐字节长度最严格的（对齐字节数最大）作为分配内存的对齐字节长度
    std::aligned_union_t<64, double, int, char> buffer1;
    std::cout << sizeof(buffer1) << std::endl;
    std::cout << alignof(buffer1) << std::endl;
    // 3) std::align接受一个指定大小的缓冲区空间的指针和一个对齐字节长度，返回一个该缓冲区中最近的能找到符合指定对齐字节长度的指针
    {
        alignas(16) char buffer[64];
        void *ptr = buffer; // 起始内存
        std::size_t space = sizeof(buffer);

        // 对齐到 32 字节，并为 8 字节对象找位置
        void *p = std::align(32, 8, ptr, space);

        if (p) {
            std::cout << "Aligned pointer: " << p << "\n";
            std::cout << "Remaining space: " << space << "\n";
        } else {
            std::cout << "Alignment failed\n";
        }
    }

    // 4) 现在c++多了一个new函数，对齐new:
    // void* operator new(std::size_t, std::align_val_t);
    // void* operator new[](std::size_t, std::align_val_t);
    // 不过这个参数编译器会自己传入，不需要用户手动参与
}
