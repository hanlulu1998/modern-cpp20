#include <iostream>
#include <cstdio>
#include "a.hpp"
#include "b.hpp"

// 1. 内联命名空间
namespace A {
  inline namespace B {
    inline namespace C{
      void bar(){
        
      } 
    }
      void foo(){
      }
  }
}

// 2. 嵌套使用
namespace A1::inline B1::inline C1 {
  void bar1(){
    
  }
}

// 3. inline 类中的静态变量，可以直接初始化
// 4. 对于类内部的static，由于需要外部定义，所以默认是外部链接，
// 这里和全局变量staitc默认内部链接不一样，会出现违反ODR情况
// 所以之前都是要求在.cpp文件中定义静态变量，不要在.hpp中定义
// 现在有了inline就可以合并了，保证了ODR
class Base{
  public:
    static inline int A = 1;
    const static int B = 1;
};

int main(void){
  // 1. 内联命名空间，可以省略命名空间书写
  A::foo();
  A::bar();

  A::B::foo();
  A::B::bar();
  A::B::C::bar();
  

  // 2. 嵌套使用
  A1::bar1();

  // 3. inline 类中的静态变量
  Base::A = 2;
  printf("A is %d, B is %d\n", Base::A, Base::B);


  // 4. inline保证ODR，让头文件的变量及函数等被链接器合并为一个
  // 对于函数而言，inline内联展开的原始语义没有新的变化
  printf("main.cpp:B_INLINE is %d\n", ++B_INLINE);
  printf("main.cpp:B_CONST is %d\n", B_CONST);
  printf("main.cpp:B_STATIC is %d\n",++B_STATIC);
  printf("main.cpp:B_INLINE_CONST is %d\n", B_INLINE_CONST);

  
  printf("a.cpp:B_INLINE is %d\n", get_b_inline());
  printf("a.cpp:B_CONST is %d\n", get_b_const());
  printf("a.cpp:B_STATIC is %d\n", get_b_static());
  printf("a.cpp:B_INLINE_CONST is %d\n", get_b_inline_const());
  

  set_b_inline(-10);
  set_b_static(-10);

  inline_func();
  // 运行可以知道inline只会有一个，修改时每个TU相互影响
  // static内部链接，每个TU一个，修改时只会改动各自TU的那个
  // const默认同static一样内部链接，不过一般而言，只要不取地址，编译器默认字面量不构造
  // 优先级上，static > inline > const


  
  // 类模板和函数模板天然ODR，但是变量模板不是
  // 需要inline进行ODR保证
  printf("value is %d\n",get_int_value());
  set_int_value(10);
  printf("value is %d\n",value<int>);
  value<int> = -10;
  printf("value is %d\n",get_int_value());

  return 0;
}
