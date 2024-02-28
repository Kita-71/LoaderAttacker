#ifndef _FILE_READER_H_
#define _FILE_READER_H_
#include <assert.h>
#include <windows.h>

#include <iostream>
#include <string>
class FileReader {
 private:
  HANDLE fileHandle;

 public:
  FileReader(std::string filePath) {
    fileHandle =
        CreateFileA(filePath.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, NULL);  // 打开目标文件
    assert(fileHandle != NULL);
  }
  virtual ~FileReader() { CloseHandle(fileHandle); }
};
#endif