#include "FileReader.h"
#include "gtest/gtest.h"
TEST(FILE_READER_TEST, READ) {
  FileReader fileReader("simpleProgram.exe");
  // 读取MZ字段
  // fileReader.ReadFileByOffset();
}
TEST(FILE_READER_TEST, FIND) {
  EXPECT_EQ(FileReader::IsFileExist(L"utest.pdb"), true);
  EXPECT_EQ(FileReader::IsFileExist(L"G:\\LoaderAttacker\\CMakeLists.txt"),
            true);
  FileReader::GetDllPath(L"kernel32.dll");
  // 读取MZ字段
  // fileReader.ReadFileByOffset();
}
int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}