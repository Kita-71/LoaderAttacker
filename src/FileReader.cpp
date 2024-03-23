#include "FileReader.h"

#include <string>
#include <vector>
FileReader::FileReader(std::string filePath) {
  fileHandle =
      CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);  // 打开目标文件
  // 检查打开文件是否成功
  CHECK_CONDITION((fileHandle == NULL), "Open Failed");
}
bool FileReader::ReadFileByOffset(LPVOID buffer, DWORD size, DWORD offset,
                                  std::string errorName) {
  SetFilePointer(fileHandle, offset, NULL, FILE_BEGIN);
  {
    bool ret = TRUE;
    ret = ReadFile(fileHandle, buffer, size, NULL, NULL);
    CHECK_CONDITION((ret == FALSE), (errorName + "Read Failed").c_str());
  }
}
bool FileReader::IsFileExist(std::wstring path) {
  DWORD fileAttr = GetFileAttributesW(path.c_str());
  return (fileAttr != INVALID_FILE_ATTRIBUTES &&
          !(fileAttr & FILE_ATTRIBUTE_DIRECTORY));
}
std::wstring FileReader::GetDllPath(std::wstring name) {
  std::vector<std::wstring> searchPaths;

  // 获取系统目录
  WCHAR systemDir[MAX_PATH];
  GetSystemDirectoryW(systemDir, MAX_PATH);
  searchPaths.push_back(systemDir);

  // 获取 Windows 目录
  WCHAR windowsDir[MAX_PATH];
  GetWindowsDirectoryW(windowsDir, MAX_PATH);
  searchPaths.push_back(windowsDir);

  // 获取当前工作目录
  WCHAR currentDir[MAX_PATH];
  GetCurrentDirectoryW(MAX_PATH, currentDir);
  searchPaths.push_back(currentDir);

  // 获取 PATH 环境变量
  WCHAR pathEnvVar[32767];  // 最大环境变量长度
  DWORD pathLen = GetEnvironmentVariableW(L"PATH", pathEnvVar, 32767);
  std::wstring paths(pathEnvVar, pathLen);

  size_t pos = 0;
  std::wstring delimiter = L";";
  size_t tokenPos;
  while ((tokenPos = paths.find(delimiter, pos)) != std::wstring::npos) {
    searchPaths.push_back(paths.substr(pos, tokenPos - pos));
    pos = tokenPos + delimiter.length();
  }
  searchPaths.push_back(paths.substr(pos));
  /*
  for (const auto& path : searchPaths) {
    std::wcout << path << std::endl;
  }
  */
  // 在指定的目录中查找 DLL
  for (const auto& path : searchPaths) {
    std::wstring fullPath = path + L"\\" + name;
    if (IsFileExist(fullPath)) {
      std::wcout << L"FIND DLL: " << fullPath << std::endl;
      return fullPath;
    }
  }

  std::wcout << L"CAN NOT FIND DLL: " << name << std::endl;
  return false;
}