#include "a.hpp"
#include "b.hpp"


int get_b_inline(){
  return B_INLINE;
}

void set_b_inline(int a){
  B_INLINE = a;
}

int get_b_const(){
  return B_CONST;
}


void set_b_static(int a){
  B_STATIC = a;
}

int get_b_static(){
  return B_STATIC;
}

int get_b_inline_const(){
  return B_INLINE_CONST;
}

void set_int_value(int a){
  value<int> = a;
}

int get_int_value(){
  return value<int>;
}
