#include "ImgManager.h"

#include <iostream>

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
    // std::cout << "## CREATE PE IMG,BASE:" << (DWORD)imgBuffer << std::endl;
  }
  // else
  //  imgArray.push_back(newItem);
  return true;
}

bool ImgManager::Relocate(ImgItem* item) {
  // 获取重定位表的Virtual Address
  DWORD relAddress = item->GetRelVirtualAddress();
  if (relAddress == 0) return false;
  PIMAGE_BASE_RELOCATION relTableBase =
      (PIMAGE_BASE_RELOCATION)(item->GetImgBase() + relAddress);
  PIMAGE_BASE_RELOCATION curRelBlock = relTableBase;
  while (curRelBlock->SizeOfBlock != 0 && curRelBlock->VirtualAddress != 0) {
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
  std::cout << "## RELOCATE" << std::endl;
  return true;
};

bool ImgManager::FixImportTable(ImgItem* item) {
  // 找到导入表
  DWORD importTableOffset = item->GetImpVirtualAddress();
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
    char* name = (char*)(nameAddress);

    // TODO：查看动态库是否在内存，不在则加载动态库
    /*查找自己的dll管理器
    std::string Sname = name;
    ImgItem* item = GetItemByName(name);
    if (item == NULL) {
      // 加载
      ;
    }*/
    // 先完成假设已经在内存里的内容

    // 获取dll的base
    HMODULE dllHandle = GetModuleHandleA(name);
    if (NULL == dllHandle) {
      dllHandle = LoadLibraryA(name);
      if (NULL == dllHandle) {
        curImpTableItem++;
        continue;
      }
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
      FARPROC funcAddress;
      if (INTItem[i].u1.AddressOfData & 0x80000000) {
        // 序号导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress = GetProcAddress(
            dllHandle, (LPCSTR)(INTItem[i].u1.Ordinal & 0x0000FFFF));
      } else {
        // 名称导出
        // TODO：获取导出函数在内存中的地址（查找dll基址和导出函数表）
        funcAddress = GetProcAddress(dllHandle, (LPCSTR)importByName->Name);
      }
      IATItem[i].u1.Function = (DWORD)funcAddress;
      i++;
    }
    curImpTableItem++;
  }
  std::cout << "## FIXING IMPORT TABLE" << std::endl;
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
  // std::wstring path = FileReader::GetDllPath(StringCov::toWideString(name));
  return 0;
}
