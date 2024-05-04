#ifndef _EXP_TABLE_READER_
#define _EXP_TABLE_READER_
#include <Windows.h>

#include "Img.h"
class ExpTableReader {
 private:
 public:
  static DWORD GetFuncAddr(ImgItem* dllItem, int impWay, const char* nameOrId);
};
#endif