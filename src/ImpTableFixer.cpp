#include "ImpTableFixer.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "errorMsg.h"

DWORD ImpTableFixer::GetFuncAddr(ImgItem* dllItem, int impWay,
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
  if (impWay == 0) {
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
      char* stubFuncName = (char*)malloc(strlen(nameOrId) + 5);
      memcpy(stubFuncName, nameOrId, strlen(nameOrId));
      memcpy(stubFuncName + strlen(nameOrId), "Stub", 5);
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

bool ImpTableFixer::FixImportTable(ImgManager* imgManager, ImgItem* item) {
  std::cout << "## FIX IMPORT TABLE START" << std::endl;
  // 找到导入表
  DWORD importTableOffset = item->GetImpVirtualAddress();
  if (importTableOffset == 0) {
    std::cout << "## IMPORT TABLE NOT EXIST,FINISH" << std::endl;
    return false;
  }
  PIMAGE_IMPORT_DESCRIPTOR importTableBase =
      (PIMAGE_IMPORT_DESCRIPTOR)((DWORD)item->GetImgBase() + importTableOffset);
  PIMAGE_IMPORT_DESCRIPTOR curImpTableItem = importTableBase;

  // 对此导入表进行处理
  while (curImpTableItem->Characteristics != 0) {
    // 获取INT表、IAT表、name的地址
    DWORD INTAddress =
        (DWORD)item->GetImgBase() + curImpTableItem->OriginalFirstThunk;
    DWORD IATAddress = (DWORD)item->GetImgBase() + curImpTableItem->FirstThunk;
    DWORD nameAddress = (DWORD)item->GetImgBase() + curImpTableItem->Name;
    DWORD dllBase;
    char* name = (char*)(nameAddress);
    if (strcmp(name, "KERNEL32.dll") == 0) memcpy(name + 9, "DLL", 3);
    dllBase = imgManager->GetDllBase(name);
    if (NULL == dllBase) {
      curImpTableItem++;
      continue;
    }
    // INT表的item
    PIMAGE_THUNK_DATA INTItem = (PIMAGE_THUNK_DATA)INTAddress;
    PIMAGE_THUNK_DATA IATItem = (PIMAGE_THUNK_DATA)IATAddress;
    int i = 0;
    while (INTItem[i].u1.AddressOfData != 0) {
      DWORD importByNameAddress =
          (DWORD)item->GetImgBase() + INTItem[i].u1.AddressOfData;
      PIMAGE_IMPORT_BY_NAME importByName =
          (PIMAGE_IMPORT_BY_NAME)importByNameAddress;
      DWORD funcAddress;
      std::unordered_map<std::string, ImgItem*>& imgArray =
          imgManager->GetImgArray();
      if (INTItem[i].u1.AddressOfData & 0x80000000) {
        // 序号导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress = GetFuncAddr(imgArray[name], 0,
                                  (LPCSTR)(INTItem[i].u1.Ordinal & 0x0000FFFF));
      } else {
        // 名称导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress =
            GetFuncAddr(imgArray[name], 1, (LPCSTR)importByName->Name);
        if (funcAddress >= imgArray[name]->GetImgBase() +
                               imgArray[name]->GetExpVirtualAddress() &&
            funcAddress <= imgArray[name]->GetImgBase() +
                               imgArray[name]->GetExpVirtualAddress() +
                               imgArray[name]->GetExpSize()) {
          std::string relString = (char*)funcAddress;
          int i = relString.find('.');
          std::string relName = relString.substr(0, i) + ".dll";
          transform(relName.begin(), relName.end(), relName.begin(), ::tolower);
          std::string relFuncName =
              relString.substr(i + 1, relString.size() - i - 1);

          DWORD relDllBse = imgManager->GetDllBase(relName.c_str());
          funcAddress =
              GetFuncAddr(imgArray[relName], 1, (LPCSTR)relFuncName.c_str());
        }
        CHECK_CONDITION(funcAddress == 0, "FUNC GET FAILED");
      }
      IATItem[i].u1.Function = (DWORD)funcAddress;
      i++;
    }
    curImpTableItem++;
  }
  std::cout << "## FIX IMPORT TABLE FINISH" << std::endl;
  return true;
};