// library.cpp
#include "testdll.h"

#include <iostream>
__declspec(dllexport) void hello() {
  // 实现函数
  std::cout << "Extern Function Call Success" << std::endl;
}