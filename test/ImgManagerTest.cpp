#include <iostream>

#include "ImgManager.h"
#include "ImpTableFixer.h"
#include "Relocator.h"
#include "gtest/gtest.h"
TEST(IMG_MANAGER_TEST, CREATE_HEADERS) {
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetExeItem();
  // GetImgHeaders
  ImgHeaders* imgHeaders = item->GetImgHeaders();
  // DOS头测试
  PIMAGE_DOS_HEADER dosHeader = imgHeaders->GetDosHeader();
  EXPECT_EQ(dosHeader->e_magic, 0x5A4D);  //"MZ"
  // NT头测试
  IMAGE_NT_HEADERS* ntHeader = imgHeaders->GetNtHeader32();
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
  ImgItem* item = imgManager.GetExeItem();
  // SECTION_TABLE
  DWORD num = item->GetSectionNum();
  EXPECT_EQ(num, 7);
  DWORD secTableSize = item->GetSectionTableSize();
  EXPECT_EQ(secTableSize, 0x118);

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
  ImgItem* item = imgManager.GetExeItem();
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