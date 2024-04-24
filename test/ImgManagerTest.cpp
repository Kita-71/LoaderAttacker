#include <iostream>

#include "ImgManager.h"
#include "ImpTableFixer.h"
#include "Relocator.h"
#include "gtest/gtest.h"
TEST(IMG_MANAGER_TEST, CREATE_HEADERS) {
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetPeItem();
  // GetImgHeaders
  ImgHeaders* imgHeaders = item->GetImgHeaders();
  // DOS头测试
  PIMAGE_DOS_HEADER dosHeader = imgHeaders->GetDosHeader();
  EXPECT_EQ(dosHeader->e_magic, 0x5A4D);  //"MZ"
  // NT头测试
  IMAGE_NT_HEADERS* ntHeader = imgHeaders->GetNtHeader();
  EXPECT_EQ(ntHeader->Signature, 0x4550);  //"PE"
  // 各种Get测试
  DWORD imgSize = item->GetImageSize();
  EXPECT_EQ(imgSize, 0x11000);
  DWORD headersSize = item->GetHeadersSize();
  EXPECT_EQ(headersSize, 0x400);
}
TEST(IMG_MANAGER_TEST, CREATE_SECTION_HEADER) {
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetPeItem();
  // SECTION_TABLE
  DWORD num = item->GetSectionNum();
  EXPECT_EQ(num, 8);
  DWORD secTableSize = item->GetSectionTableSize();
  EXPECT_EQ(secTableSize, 0x140);

  // 遍历Section Table
  IMAGE_SECTION_HEADER* secHeader = item->GetSectionHeader();
  EXPECT_EQ(strcmp((const char*)secHeader[0].Name, ".text"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[1].Name, ".rdata"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[2].Name, ".data"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[3].Name, ".idata"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[4].Name, ".00cfg"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[5].Name, ".rsrc"), 0);
  EXPECT_EQ(strcmp((const char*)secHeader[6].Name, ".reloc"), 0);
}
TEST(IMG_MANAGER_TEST, CREATE_SECTION) {
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetPeItem();
  char* buffer = (char*)item->GetImgBase();
  // text段
  EXPECT_EQ(*((BYTE*)(buffer + 0x1000)), 0xCC);
  EXPECT_EQ(*((BYTE*)(buffer + 0x1001)), 0xCC);
  EXPECT_EQ(*((BYTE*)(buffer + 0x1002)), 0xCC);
  EXPECT_EQ(*((BYTE*)(buffer + 0x1003)), 0xCC);
  EXPECT_EQ(*((BYTE*)(buffer + 0x1004)), 0xCC);
  EXPECT_EQ(*((BYTE*)(buffer + 0x1005)), 0xE9);
  // data段
  EXPECT_EQ(*((BYTE*)(buffer + 0xB000)), 0x4E);
  EXPECT_EQ(*((BYTE*)(buffer + 0xB001)), 0xE6);
  EXPECT_EQ(*((BYTE*)(buffer + 0xB002)), 0x40);
  EXPECT_EQ(*((BYTE*)(buffer + 0xB003)), 0xBB);
  EXPECT_EQ(*((BYTE*)(buffer + 0xB004)), 0xB1);
  EXPECT_EQ(*((BYTE*)(buffer + 0xB005)), 0x19);
}
TEST(IMG_MANAGER_TEST, RELOCATE) {
  Relocator relocator;
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetPeItem();
  char* buffer = (char*)item->GetImgBase();
  EXPECT_EQ(item->GetRelVirtualAddress(), 0x10000);
  EXPECT_EQ(item->GetFileExpectedBase(), 0x400000);
  // 确认找到重定位表
  EXPECT_EQ(*((DWORD*)(buffer + 0x10000)), 0x00001000);
  EXPECT_EQ(*((DWORD*)(buffer + 0x10004)), 0x0000007C);
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x0517)), 0x00401028);
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x051C)), 0x00408C44);
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x0521)), 0x0040C0D4);
  relocator.Relocate(item);
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x0517)),
            0x00401028 + (DWORD)(buffer - 0x00400000));
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x051C)),
            0x00408C44 + (DWORD)(buffer - 0x00400000));
  EXPECT_EQ(*((DWORD*)(buffer + 0x00001000 + 0x0521)),
            0x0040C0D4 + (DWORD)(buffer - 0x00400000));
}
TEST(IMG_MANAGER_TEST, IMP_TABLE) {
  ImgManager imgManager;
  ImpTableFixer impTableFixer;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetPeItem();
  char* buffer = (char*)item->GetImgBase();
  EXPECT_EQ(item->GetImpVirtualAddress(), 0xC240);
  // 四个导入表
  EXPECT_EQ(*((DWORD*)(buffer + 0xC240)), 0xC33C);
  EXPECT_EQ(*((DWORD*)(buffer + 0xC240 + 20)), 0xC3B8);
  EXPECT_EQ(*((DWORD*)(buffer + 0xC240 + 40)), 0xC40C);
  EXPECT_EQ(*((DWORD*)(buffer + 0xC240 + 60)), 0xC2A4);
  // 检查各个导入表的名称
  EXPECT_EQ(strcmp((char*)buffer + 0xC8D6, "MSVCP140D.dll"), 0);
  EXPECT_EQ(strcmp((char*)buffer + 0xC9C0, "VCRUNTIME140D.dll"), 0);
  EXPECT_EQ(strcmp((char*)buffer + 0xCC60, "ucrtbased.dll"), 0);
  EXPECT_EQ(strcmp((char*)buffer + 0xCE44, "KERNEL32.dll"), 0);
  // 测试函数地址是否已被修改
  /// 1.MSVCP140D.dll 的 flush
  EXPECT_EQ(*((DWORD*)(buffer + 0xC33C)), 0xC858);
  EXPECT_EQ(
      strcmp((char*)(buffer + 0xC85A),
             "?flush@?$basic_ostream@DU?$char_traits@D@std@@@std@@QAEAAV12@XZ"),
      0);
  /// 2.KERNEL32.dll 的
  EXPECT_EQ(*((DWORD*)(buffer + 0xC2A4)), 0xCE02);
  EXPECT_EQ(strcmp((char*)(buffer + 0xCE04), "GetProcessHeap"), 0);

  // 查看
  impTableFixer.FixImportTable(&imgManager, item);
  // std::cout << (DWORD)GetModuleHandle("MSVCP140D.dll") << std::endl;
  // std::cout << *((DWORD*)(buffer + 0xC338)) << std::endl;
}
TEST(IMG_MANAGER_TEST, EXE) {}
