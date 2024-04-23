#ifndef _FILE_READER_H_
#define _FILE_READER_H_
#include <windows.h>

#include <iostream>
#include <string>

#include "errorMsg.h"
class FileReader {
 private:
  HANDLE fileHandle;

 public:
  FileReader(std::string filePath);
  virtual ~FileReader() { CloseHandle(fileHandle); }
  bool ReadFileByOffset(LPVOID buffer, DWORD size, DWORD offset,
                        std::string errorName);
  inline HANDLE GetHandle() { return fileHandle; }
  static bool IsFileExist(std::wstring path);
  static std::wstring FileReader::GetDllPath(std::wstring name);
  static std::string GetNameByPath(std::string path);
  static std::string GetFPathByPath(std::string path);
};
#endif