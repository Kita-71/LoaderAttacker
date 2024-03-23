#ifndef _TERMINAL_H_
#define _TERMINAL_H_
#include <iostream>
class Terminal {
 private:
  /* data */
 public:
  Terminal(/* args */);
  ~Terminal();
  static void ShowHelp() { std::cout << "HELP" << std::endl; };
};

#endif