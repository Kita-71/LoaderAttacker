#ifndef _IMG_MANAGER_H_
#define _IMG_MANAGER_H_
#include <Windows.h>

#include <unordered_map>
#include <vector>

#include "errorMsg.h"
class ImgHeaders {
 private:
  PIMAGE_DOS_HEADER dosHeader;
  PIMAGE_NT_HEADERS ntHeader;
  PIMAGE_SECTION_HEADER secHeaders;
  DWORD secNum;

 public:
  // getter&setter
  PIMAGE_DOS_HEADER GetDosHeader() const { return dosHeader; }
  void SetDosHeader(PIMAGE_DOS_HEADER dosHeader) {
    this->dosHeader = dosHeader;
  }

  PIMAGE_NT_HEADERS GetNtHeader() const { return ntHeader; }
  void SetNtHeader(PIMAGE_NT_HEADERS ntHeader) { this->ntHeader = ntHeader; }

  PIMAGE_SECTION_HEADER GetSecHeaders() const { return secHeaders; }
  void SetSecHeaders(PIMAGE_SECTION_HEADER secHeaders) {
    this->secHeaders = secHeaders;
  }

  DWORD GetSecNum() const { return secNum; }
  void SetSecNum(DWORD secNum) { this->secNum = secNum; }
};
class ImgItem {
 private:
  DWORD imgBase;
  DWORD imgSize;
  bool createByMe;
  ImgHeaders imgHeaders;

 public:
  ImgItem(DWORD imgBase, bool createByMe) {
    CHECK_CONDITION((imgBase == NULL), "imgBase not exist");
    this->imgBase = imgBase;
    this->createByMe = createByMe;

    DWORD dosHeaderAddress = imgBase;
    imgHeaders.SetDosHeader((PIMAGE_DOS_HEADER)dosHeaderAddress);
    DWORD ntHeaderAddress = imgBase + imgHeaders.GetDosHeader()->e_lfanew;
    imgHeaders.SetNtHeader((PIMAGE_NT_HEADERS)ntHeaderAddress);
    DWORD secNum = imgHeaders.GetNtHeader()->FileHeader.NumberOfSections + 1;
    imgHeaders.SetSecNum(secNum);
    DWORD imgHeadersAddress = imgBase + imgHeaders.GetDosHeader()->e_lfanew +
                              sizeof(IMAGE_NT_HEADERS);
    imgHeaders.SetSecHeaders((PIMAGE_SECTION_HEADER)imgHeadersAddress);
    this->imgSize = imgHeaders.GetNtHeader()->OptionalHeader.SizeOfImage;
  }
  virtual ~ImgItem(){};
  // 测试用，获取头部
  ImgHeaders* GetImgHeaders() { return &imgHeaders; };
  // img、header相关
  inline DWORD GetImageSize() {
    return imgHeaders.GetNtHeader()->OptionalHeader.SizeOfImage;
  }
  inline DWORD GetHeadersSize() {
    return imgHeaders.GetNtHeader()->OptionalHeader.SizeOfHeaders;
  }
  // SectionTable相关
  inline DWORD GetSectionTableSize() { return (imgHeaders.GetSecNum()) * 0x28; }
  inline DWORD GetSectionNum() { return imgHeaders.GetSecNum(); }
  inline PIMAGE_SECTION_HEADER GetSectionHeader() {
    return imgHeaders.GetSecHeaders();
  }
  inline DWORD GetSecHeaderOffset() {
    return imgHeaders.GetDosHeader()->e_lfanew + sizeof(IMAGE_NT_HEADERS);
  }
  inline DWORD GetSectionHeaderSize() { return 0x28; }

  // 重定位相关信息
  inline DWORD GetRelVirtualAddress() {
    return imgHeaders.GetNtHeader()
        ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
        .VirtualAddress;
  }
  inline DWORD GetFileExpectedBase() {
    return imgHeaders.GetNtHeader()->OptionalHeader.ImageBase;
  }

  // Import Table相关
  inline DWORD GetImpVirtualAddress() {
    return imgHeaders.GetNtHeader()
        ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
        .VirtualAddress;
  }
  inline DWORD GetImgBase() { return this->imgBase; };
  inline void SetImgBase() {
    imgHeaders.GetNtHeader()->OptionalHeader.ImageBase = imgBase;
  }
  // 启动相关
  inline DWORD GetEntryAddress() {
    return imgHeaders.GetNtHeader()->OptionalHeader.AddressOfEntryPoint;
  }
};

class ImgManager {
 private:
  ImgItem* peItem;
  std::unordered_map<std::string, ImgItem*> imgArray;

 public:
  inline ImgItem* GetPeItem() { return peItem; }
  ImgManager() {
    peItem = NULL;
    // 将已加载的模块初始化到itemArray中
  };
  virtual ~ImgManager(){};
  bool AddExistedDll();
  bool CreateImgArea(std::string path, bool isPE);
  bool Relocate(ImgItem* item);
  bool FixImportTable(ImgItem* item);
  ImgItem* GetItemByName(std::string name);
  DWORD DllLoader(std::string name);
  bool CallEntry();
};
#endif