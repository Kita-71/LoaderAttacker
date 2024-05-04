#ifndef _IMG_H_
#define _IMG_H_
#include <Windows.h>
#include <tlhelp32.h>

#include <string>

#include "errorMsg.h"
class ImgHeaders {
 private:
  PIMAGE_DOS_HEADER dosHeader;       // dos头部
  PIMAGE_NT_HEADERS32 ntHeader32;    // 32位nt头部
  PIMAGE_NT_HEADERS64 ntHeader64;    // 64位nt头部
  PIMAGE_SECTION_HEADER secHeaders;  // 节表位置
  DWORD secNum;                      // 节数
 public:
  // getter&setter
  PIMAGE_DOS_HEADER GetDosHeader() const { return dosHeader; }
  void SetDosHeader(PIMAGE_DOS_HEADER dosHeader) {
    this->dosHeader = dosHeader;
  }

  PIMAGE_NT_HEADERS32 GetNtHeader32() const { return ntHeader32; }
  void SetNtHeader32(PIMAGE_NT_HEADERS32 ntHeader32) {
    this->ntHeader32 = ntHeader32;
  }
  PIMAGE_NT_HEADERS64 GetNtHeader64() const { return ntHeader64; }
  void SetNtHeader64(PIMAGE_NT_HEADERS64 ntHeader64) {
    this->ntHeader64 = ntHeader64;
  }

  PIMAGE_SECTION_HEADER GetSecHeaders() const { return secHeaders; }
  void SetSecHeaders(PIMAGE_SECTION_HEADER secHeaders) {
    this->secHeaders = secHeaders;
  }

  DWORD GetSecNum() const { return secNum; }
  void SetSecNum(DWORD secNum) { this->secNum = secNum; }
};

class ImgItem {
 private:
  DWORD imgBase;          // 镜像加载基址
  DWORD imgSize;          // 镜像大小
  bool createByMe;        // 标志该PE是否由该加载器加载
  ImgHeaders imgHeaders;  // PE文件头部信息
  std::string name;       // PE文件名
  std::string path;       // PE文件所在路径
  bool is64;              // 标志此PE文件是32位还是64位

 public:
  std::string GetName() const { return name; }
  void SetName(std::string name) { this->name = name; }
  std::string GetPath() const { return path; }
  void SetPath(std::string path) { this->path = path; }

  ImgItem(DWORD imgBase, bool createByMe) {
    CHECK_CONDITION((imgBase == NULL), "imgBase not exist");
    this->imgBase = imgBase;
    this->createByMe = createByMe;

    DWORD dosHeaderAddress = imgBase;
    imgHeaders.SetDosHeader((PIMAGE_DOS_HEADER)dosHeaderAddress);
    DWORD ntHeaderAddress = imgBase + imgHeaders.GetDosHeader()->e_lfanew;
    imgHeaders.SetNtHeader32((PIMAGE_NT_HEADERS32)ntHeaderAddress);
    imgHeaders.SetNtHeader64((PIMAGE_NT_HEADERS64)ntHeaderAddress);
    DWORD secNum = imgHeaders.GetNtHeader32()->FileHeader.NumberOfSections;
    imgHeaders.SetSecNum(secNum);
    DWORD imgHeadersAddress =
        (DWORD)IMAGE_FIRST_SECTION((PIMAGE_NT_HEADERS)ntHeaderAddress);
    imgHeaders.SetSecHeaders((PIMAGE_SECTION_HEADER)imgHeadersAddress);
    this->imgSize = imgHeaders.GetNtHeader32()->OptionalHeader.SizeOfImage;
    if (imgHeaders.GetNtHeader32()->OptionalHeader.Magic == 0x10B)
      is64 = false;
    else
      is64 = true;
  }
  virtual ~ImgItem(){};
  // 测试用，获取头部
  ImgHeaders* GetImgHeaders() { return &imgHeaders; };
  // name和path

  // img、header相关
  inline DWORD GetImageSize() {
    return (is64 ? (imgHeaders.GetNtHeader64()->OptionalHeader.SizeOfImage)
                 : (imgHeaders.GetNtHeader32()->OptionalHeader.SizeOfImage));
  }
  inline DWORD GetHeadersSize() {
    return (is64 ? (imgHeaders.GetNtHeader64()->OptionalHeader.SizeOfHeaders)
                 : (imgHeaders.GetNtHeader32()->OptionalHeader.SizeOfHeaders));
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
    return (is64 ? (imgHeaders.GetNtHeader64()
                        ->OptionalHeader
                        .DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
                        .VirtualAddress)
                 : (imgHeaders.GetNtHeader32()
                        ->OptionalHeader
                        .DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC]
                        .VirtualAddress));
  }
  inline DWORD GetFileExpectedBase() {
    return (is64 ? (imgHeaders.GetNtHeader64()->OptionalHeader.ImageBase)
                 : (imgHeaders.GetNtHeader32()->OptionalHeader.ImageBase));
  }

  // Import Table相关
  inline DWORD GetImpVirtualAddress() {
    return (
        is64 ? (imgHeaders.GetNtHeader64()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                    .VirtualAddress)
             : (imgHeaders.GetNtHeader32()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
                    .VirtualAddress));
  }
  inline DWORD GetImgBase() { return this->imgBase; };
  inline void SetImgBase() {
    (is64 ? (imgHeaders.GetNtHeader64()->OptionalHeader.ImageBase = imgBase)
          : (imgHeaders.GetNtHeader32()->OptionalHeader.ImageBase = imgBase));
  }
  // 导出表相关
  inline DWORD GetExpVirtualAddress() {
    return (
        is64 ? (imgHeaders.GetNtHeader64()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .VirtualAddress)
             : (imgHeaders.GetNtHeader32()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .VirtualAddress));
  }
  inline DWORD GetExpSize() {
    return (
        is64 ? (imgHeaders.GetNtHeader64()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .Size)
             : (imgHeaders.GetNtHeader32()
                    ->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
                    .Size));
  }
  // 启动相关
  inline DWORD GetEntryAddress() {
    return (
        is64
            ? (imgHeaders.GetNtHeader64()->OptionalHeader.AddressOfEntryPoint)
            : (imgHeaders.GetNtHeader32()->OptionalHeader.AddressOfEntryPoint));
  }
};
#endif