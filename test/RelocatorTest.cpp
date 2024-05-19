#include <iostream>

#include "ImgManager.h"
#include "ImpTableFixer.h"
#include "Relocator.h"
#include "gtest/gtest.h"

TEST(RELOCATOR_TEST, RELOCATE) {
  Relocator relocator;
  ImgManager imgManager;
  imgManager.CreateImgArea("simpleProgram.exe", true);
  ImgItem* item = imgManager.GetExeItem();
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