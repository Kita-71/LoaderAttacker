#ifndef _ERROR_MSG_H_
#define _ERROR_MSG_H_
#include <cassert>
#include <iostream>

#define CHECK_CONDITION(condition, msg)                           \
  do {                                                            \
    if ((condition)) {                                            \
      std::cerr << "Error: " << msg << "\n"                       \
                << "Condition: " << #condition << "\n"            \
                << "ErrorNum: " << GetLastError() << "\n"         \
                << "File: " << __FILE__ << ", Line: " << __LINE__ \
                << std::endl;                                     \
      std::exit(EXIT_FAILURE);                                    \
    }                                                             \
  } while (false)
#endif