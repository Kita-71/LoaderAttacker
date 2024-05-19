#include <iostream>

#include "ImgManager.h"
#include "ImpTableFixer.h"
#include "Relocator.h"
#include "gtest/gtest.h"
TEST(IMP_TABLE_TEST, FIX_IMP_TABLE) {
  ImgManager imgManager;
  ImpTableFixer impTableFixer;
  ApiMsReader apiMsReader;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetExeItem();
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
  impTableFixer.FixImportTable(&imgManager, &apiMsReader, item);
  // std::cout << (DWORD)GetModuleHandle("MSVCP140D.dll") << std::endl;
  // std::cout << *((DWORD*)(buffer + 0xC338)) << std::endl;
}