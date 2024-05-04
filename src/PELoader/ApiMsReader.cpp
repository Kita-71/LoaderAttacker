#include "ApiMsReader.h"

#include <windows.h>

#include <iostream>

#include "FileReader.h"
#include "Img.h"
#include "StringCov.h"

void ApiMsReader::GetApiSet() {
  std::wstring path =
      FileReader::GetDllPath(StringCov::toWideString("apisetschema.dll"));
  FileReader fileReader(StringCov::toByteString(path));
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
  apiMsDllBase = (DWORD)imgBuffer;

  // 读取img各个头部
  DWORD headersSize = ntHeader.OptionalHeader.SizeOfHeaders;
  fileReader.ReadFileByOffset(imgBuffer, headersSize, 0, "AllHeaders");

  // 创建Item
  ImgItem* newItem = new ImgItem((DWORD)imgBuffer, true);
  // 加载 Section
  DWORD secNum = newItem->GetSectionNum();
  IMAGE_SECTION_HEADER* pSecTable = newItem->GetSectionHeader();
  IMAGE_SECTION_HEADER* pCurSec = pSecTable;
  for (DWORD i = 0; i < secNum; i++) {
    if (strcmp((const char*)pCurSec->Name, ".apiset") == 0) {
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
      apiSectionBase = (DWORD)(imgBuffer + virtualAddress);
      return;
    }
    pCurSec++;
  }
}
void ApiMsReader::ParseApiSet() {
  PAPI_SET_NAMESPACE pasn = (PAPI_SET_NAMESPACE)apiSectionBase;
  PAPI_SET_NAMESPACE_ENTRY pasnEntryTable =
      (PAPI_SET_NAMESPACE_ENTRY)(pasn->EntryOffset + (DWORD)pasn);
  PAPI_SET_NAMESPACE_ENTRY pasnCurEntry = pasnEntryTable;
  unsigned int count = pasn->Count;
  for (unsigned int i = 0; i < count; i++) {
    // key
    UNICODE_STRING originName;
    originName.Buffer = (PWSTR)(pasnCurEntry->NameOffset + (DWORD)pasn);
    originName.Length = pasnCurEntry->NameLength;
    originName.MaximumLength = pasnCurEntry->NameLength;

    char* buffer = (char*)malloc(originName.Length + 1);
    StringCov::UnicodeToChar(&originName, buffer);
    std::string originNameString = buffer;
    originNameString += ".dll";

    // value
    PAPI_SET_VALUE_ENTRY pasvEntry =
        (PAPI_SET_VALUE_ENTRY)(pasnCurEntry->ValueOffset + (DWORD)pasn);
    UNICODE_STRING forwardName;
    forwardName.Buffer = (PWSTR)(pasvEntry->ValueOffset + (DWORD)pasn);
    forwardName.Length = pasvEntry->ValueLength;
    forwardName.MaximumLength = pasvEntry->ValueLength;

    char* buffer2 = (char*)malloc(forwardName.Length + 1);
    StringCov::UnicodeToChar(&forwardName, buffer2);
    std::string forwardNameString = buffer2;
    apiMsMap[originNameString] = forwardNameString;
    pasnCurEntry++;
  }
}

ApiMsReader::ApiMsReader() {
  GetApiSet();
  ParseApiSet();
}