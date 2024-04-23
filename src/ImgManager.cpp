#include "ImgManager.h"

#include <algorithm>
#include <iostream>
#include <string>

#include "FileReader.h"
#include "StringCov.h"
bool ImgManager::CreateImgArea(std::string path, bool isPE) {
  if (isPE && peItem != NULL) return false;
  // 打开文件
  FileReader fileReader(path);
  // 读取ntHeader和dosHeader
  IMAGE_DOS_HEADER dosHeader;
  fileReader.ReadFileByOffset(&dosHeader, sizeof(dosHeader), 0, "DOS HEADER");
  IMAGE_NT_HEADERS ntHeader;
  fileReader.ReadFileByOffset(&ntHeader, sizeof(ntHeader), dosHeader.e_lfanew,
                              "NT HEADER");
  // 读取imgSize
  DWORD imgSize = ntHeader.OptionalHeader.SizeOfImage;
  // 创建img空间
  char* imgBuffer = (char*)VirtualAlloc(NULL, imgSize, MEM_COMMIT | MEM_RESERVE,
                                        PAGE_EXECUTE_READWRITE);

  // 读取img各个头部
  DWORD headersSize = ntHeader.OptionalHeader.SizeOfHeaders;
  fileReader.ReadFileByOffset(imgBuffer, headersSize, 0, "AllHeaders");
  // 创建imgItem
  ImgItem* newItem = new ImgItem((DWORD)imgBuffer, true);
  newItem->SetName(FileReader::GetNameByPath(path));
  if (FileReader::GetFPathByPath(path) == "") {
    WCHAR currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    std::wstring ws = currentDir;
    newItem->SetPath(StringCov::toByteString(ws));
  } else
    newItem->SetPath(FileReader::GetFPathByPath(path));
  // 加载 Section
  DWORD secNum = newItem->GetSectionNum();
  IMAGE_SECTION_HEADER* pSecTable = newItem->GetSectionHeader();
  IMAGE_SECTION_HEADER* pCurSec = pSecTable;
  for (DWORD i = 0; i < secNum; i++) {
    DWORD virtualAddress = pCurSec->VirtualAddress;
    DWORD rawDataSize = pCurSec->SizeOfRawData;
    DWORD pointerToRaw = pCurSec->PointerToRawData;
    if (virtualAddress == 0 || rawDataSize == 0) {
      pCurSec++;
      continue;
    }
    // 拷贝当前section
    fileReader.ReadFileByOffset(imgBuffer + virtualAddress, rawDataSize,
                                pointerToRaw, "Section " + i);
    pCurSec++;
  }
  // 放入Manager
  if (isPE) {
    peItem = newItem;
  } else {
    std::string name = FileReader::GetNameByPath(path);
    imgArray[name] = newItem;
  }
  return true;
}

bool ImgManager::Relocate(ImgItem* item) {
  // 获取重定位表的Virtual Address
  std::cout << "## RELOCATE START" << std::endl;
  DWORD relAddress = item->GetRelVirtualAddress();
  if (relAddress == 0) {
    std::cout << "## RELOCATE NOT EXIST,FINISH" << std::endl;
    return false;
  }
  PIMAGE_BASE_RELOCATION relTableBase =
      (PIMAGE_BASE_RELOCATION)(item->GetImgBase() + relAddress);
  PIMAGE_BASE_RELOCATION curRelBlock = relTableBase;
  // BUG点1，先判VA，再判SB
  while (curRelBlock->VirtualAddress != 0 && curRelBlock->SizeOfBlock != 0) {
    WORD* relBlockData =
        (WORD*)((DWORD)curRelBlock + sizeof(IMAGE_BASE_RELOCATION));
    int dataCount = (curRelBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
                    sizeof(WORD);
    for (int i = 0; i < dataCount; i++) {
      if ((relBlockData[i] & 0xF000) == 0x3000)  // 标记该地址需要重定位
      {
        DWORD* pointerToData =
            (DWORD*)(item->GetImgBase() + curRelBlock->VirtualAddress +
                     (relBlockData[i] & 0x0FFF));
        DWORD imgExpectBase = item->GetFileExpectedBase();
        DWORD offset = (DWORD)item->GetImgBase() - imgExpectBase;
        *pointerToData += offset;
      }
    }
    curRelBlock =
        (PIMAGE_BASE_RELOCATION)((DWORD)curRelBlock + curRelBlock->SizeOfBlock);
  }
  std::cout << "## RELOCATE FINISH" << std::endl;
  return true;
};

bool ImgManager::FixImportTable(ImgItem* item) {
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
    dllBase = GetDllBase(name);
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

          DWORD relDllBse = GetDllBase(relName.c_str());
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

bool ImgManager::CallEntry() {
  char* ExeEntry = (char*)(peItem->GetImgBase() + peItem->GetEntryAddress());
  // 修改base
  peItem->SetImgBase();
  std::cout << "## CALL ENTRY" << std::endl;
  // 设置页属性
  DWORD dwOldProtect = 0;
  VirtualProtect((char*)peItem->GetImgBase(), peItem->GetImageSize(),
                 PAGE_EXECUTE_READWRITE, &dwOldProtect);
  // 跳转到入口点处执行
  __asm
      {
		mov eax, ExeEntry
		jmp eax
      }
  std::cout << "## CALL ENTRY SUCCESS" << std::endl;
  return true;
}

ImgItem* ImgManager::GetItemByName(std::string name) { return imgArray[name]; }
DWORD ImgManager::DllLoader(std::string name) {
  // 找dll在哪儿
  std::wstring path = FileReader::GetDllPath(StringCov::toWideString(name));
  if (path.compare(L"WRONG") == 0) {
    return 0;
  } else {
    // 找到则加载
    CreateImgArea(StringCov::toByteString(path), false);
    Relocate(imgArray[name]);
    FixImportTable(imgArray[name]);
    return imgArray[name]->GetImgBase();
  }
  return 0;
}
DWORD ImgManager::GetFuncAddr(ImgItem* dllItem, int impWay,
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

DWORD ImgManager::GetDllBase(const char* name) {
  DWORD dllBase = (DWORD)GetModuleHandle(name);
  if (NULL == dllBase) {
    WCHAR currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    std::wstring oldWorkPath = currentDir;
    dllBase = DllLoader(name);
  }
  return dllBase;
}
