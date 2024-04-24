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

DWORD ImgManager::DllLoader(std::string name) {
  // 找dll在哪儿
  std::wstring path = FileReader::GetDllPath(StringCov::toWideString(name));
  if (path.compare(L"WRONG") == 0) {
    return 0;
  } else {
    // 找到则加载
    CreateImgArea(StringCov::toByteString(path), false);
    imgQueue.push(imgArray[name]);
    return imgArray[name]->GetImgBase();
  }
  return 0;
}
DWORD ImgManager::GetDllBase(const char* name) {
  DWORD dllBase = (DWORD)imgArray[name];
  if (NULL == dllBase) {
    WCHAR currentDir[MAX_PATH];
    GetCurrentDirectoryW(MAX_PATH, currentDir);
    std::wstring oldWorkPath = currentDir;
    dllBase = DllLoader(name);
  }
  return dllBase;
}
