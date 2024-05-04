#ifndef _IMP_TABLE_FIXER_H_
#define _IMP_TABLE_FIXER_H_
#include "Img.h"
#include "ImgManager.h"
#include "ApiMsReader.h"
class ImpTableFixer {
 private:
  DWORD GetFuncAddr(ImgItem* dllItem, int impWay, const char* nameOrId);

 public:
  bool FixImportTable(ImgManager* imgManager, ApiMsReader* apiMsReader, ImgItem* item);
};
#endif