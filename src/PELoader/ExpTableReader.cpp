#include "ExpTableReader.h"
DWORD ExpTableReader::GetFuncAddr(ImgItem* dllItem, int expWay,
                                  const char* nameOrId) {
  if (!dllItem || !nameOrId) {
    return NULL;
  }
  // 获取基址
  DWORD base = dllItem->GetImgBase();
  // 获取导出表
  PIMAGE_EXPORT_DIRECTORY exportDirectory =
      (PIMAGE_EXPORT_DIRECTORY)((BYTE*)base + dllItem->GetExpVirtualAddress());

  // 获取名称指针数组、序号数组和地址数组
  DWORD* nameTablePtrs =
      (DWORD*)((BYTE*)base + exportDirectory->AddressOfNames);
  WORD* ordinalTablePtrs =
      (WORD*)((BYTE*)base + exportDirectory->AddressOfNameOrdinals);
  DWORD* addrTablePtrs =
      (DWORD*)((BYTE*)base + exportDirectory->AddressOfFunctions);
  DWORD Base = exportDirectory->Base;
  if (expWay == 0) {
    // 序号导出
    WORD ordinal = LOWORD(nameOrId);
    if (ordinal < exportDirectory->NumberOfFunctions) {
      // 根据序号找到函数地址
      return (DWORD)((BYTE*)base + addrTablePtrs[ordinal - Base]);
    }
  } else {
    // 遍历名称表，查找函数名
    for (DWORD i = 0; i < exportDirectory->NumberOfNames; ++i) {
      const char* functionName = (const char*)((BYTE*)base + nameTablePtrs[i]);
      if (strcmp(functionName, nameOrId) == 0) {
        WORD funcIndex = ordinalTablePtrs[i];
        if (funcIndex < exportDirectory->NumberOfFunctions) {
          // 根据序号找到函数地址
          return (DWORD)((BYTE*)base + addrTablePtrs[funcIndex]);
        }
      }
    }
  }
  return NULL;  // 未找到
}
