#include <iostream>
using namespace std;
void Printf() {
  cout << "Internal function calls test" << endl;
  return;
}
int main(int argc, char** argv) {
  Printf();
  return 1;
}