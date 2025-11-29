#include <climits>
#include <cstdio>
#include <iostream>
#include <limits>
#include <string>
#include <ostream>



unsigned long long  operator"" _x100(unsigned long long  a);


int main(void){
  // 1. 新的类型 long long
  long long ll = 6254ll;
  printf("printf: ll is %lld\n", ll);

  std::cout << "std::cout: ll is " << ll << std::endl;

  // 与此带来的宏
  std::cout << LLONG_MAX <<std::endl;
  std::cout <<LLONG_MIN <<std::endl;

  std::cout<<std::numeric_limits<long long>::max()<<std::endl;
  std::cout<<std::numeric_limits<long long>::min()<<std::endl;

  // 2. 新的宽字符类型
  const char16_t *char16string = u"你好";
  const char32_t *char32string = U"你好";
  // c++20强制要求utf8必须是char8_t类型
  const char8_t * char8string = u8"你好";
  // 以前旧的宽字符保留
  // 跨平台不好，不建议使用
  const wchar_t *wcharstring = L"你好";

  // 3. 新的前缀支持0b
  int a = 0b001;

  // 4. 数字支持'分割
  long l = 123'456l;
  printf("a is %d, l is %ld\n", a, l);
  
  // 5. 新的十六进制浮点格式:(1.9375) * 2^3 = 15.5
  float f = 0x1.fp3;
  printf("f is %f\n", f);

  // 5. 原始字符前缀
  std::string message =  R"raw(C:\path\(1))raw";
  std::cout << message << std::endl;

  // 6.用户自定义后缀，重载操作符<"" _xxx>
  printf("100_x100 is %llu\n", 100_x100);
  
  return 0;
}


unsigned long long  operator"" _x100(unsigned long long  a){
  return a*100;
}
