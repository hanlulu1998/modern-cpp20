#pragma once
#include <cstdio>
// 4. 定义在头文件中的变量，保证外部链接时ODR
inline int B_INLINE = 10;
// const默认为内部链接，每个单元独立
const int B_CONST = 10;
// static也是内部链接，每个单元独立
static int B_STATIC = 10;
// inline static是没有意义的，static更强
// static强制内部链接，inline只在外部链接合并时保证ODR

// inline const时，inline会覆盖const，inline更强
inline const int B_INLINE_CONST = 10;


// 4. 定义在头文件中的函数，保证外部链接ODR
inline void inline_func(){
    printf("b.hpp:B_INLINE is %d\n",B_INLINE);
    printf("b.hpp:B_CONST is %d\n", B_CONST);
    printf("b.hpp:B_STATIC is %d\n", B_STATIC);
    printf("b.hpp:B_INLINE_CONST is %d\n", B_INLINE_CONST);
}




// 4. 定义在头文件的模板变量
template<typename T>
inline T value = T{};
