#include <iostream>

#include "testdll.h"
using namespace std;
int main(int argc, char** argv) {
  cout << "Dll Test - PE Load OK" << endl;
  hello();
  return 1;
}