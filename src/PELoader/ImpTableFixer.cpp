#include "ImpTableFixer.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

#include "ExpTableReader.h"
#include "errorMsg.h"

bool ImpTableFixer::FixImportTable(ImgManager* imgManager,
                                   ApiMsReader* apiMsReader, ImgItem* item) {
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
    if (!apiMsReader->CheckDll(name).empty()) {
      std::string relName = apiMsReader->CheckDll(name);
      name = (char*)malloc(relName.size() + 1);
      memcpy(name, relName.c_str(), relName.size() + 1);
    }
    std::cout << "ImportDll:" << name << std::endl;
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
          imgManager->GetDllArray();
      if (INTItem[i].u1.AddressOfData & 0x80000000) {
        // 序号导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress = ExpTableReader::GetFuncAddr(
            imgArray[name], 0, (LPCSTR)(INTItem[i].u1.Ordinal & 0x0000FFFF));
      } else {
        // 名称导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress = ExpTableReader::GetFuncAddr(imgArray[name], 1,
                                                  (LPCSTR)importByName->Name);
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
          funcAddress = ExpTableReader::GetFuncAddr(
              imgArray[relName], 1, (LPCSTR)relFuncName.c_str());
        }
      }
      IATItem[i].u1.Function = (DWORD)funcAddress;
      i++;
    }
    curImpTableItem++;
  }
  std::cout << "## FIX IMPORT TABLE FINISH" << std::endl;
  return true;
};
