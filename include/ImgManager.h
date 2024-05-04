#ifndef _IMG_MANAGER_H_
#define _IMG_MANAGER_H_
#include <Img.h>
#include <Windows.h>
#include <tlhelp32.h>

#include <queue>
#include <unordered_map>
#include <vector>

#include "ApiMsReader.h"
#include "Relocator.h"
#include "errorMsg.h"

class ImgManager {
 private:
  ImgItem* exeItem;
  std::unordered_map<std::string, ImgItem*> dllArray;
  std::queue<ImgItem*> unHandledDllQueue;

 public:
  // 初始化、构造析构
  ImgManager() {
    exeItem = NULL;
    GetAllLoadedDll();
  };
  void GetAllLoadedDll() {
    // 将已加载的模块初始化到itemArray中
    HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
    MODULEENTRY32 me32;
    // 创建模块快照。
    hModuleSnap =
        CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
    if (hModuleSnap == INVALID_HANDLE_VALUE) {
      std::cerr << "CreateToolhelp32Snapshot failed." << std::endl;
      return;
    }
    // 初始化MODULEENTRY32结构。
    me32.dwSize = sizeof(MODULEENTRY32);
    // 遍历模块列表。
    if (Module32First(hModuleSnap, &me32)) {
      do {
        ImgItem* newItem = new ImgItem((DWORD)me32.hModule, true);
        this->dllArray[me32.szModule] = newItem;
      } while (Module32Next(hModuleSnap, &me32));
    } else {
      std::cerr << "Failed to gather module information." << std::endl;
    }

    // 清理快照对象。
    CloseHandle(hModuleSnap);
  }
  virtual ~ImgManager(){};

  // Get&Set
  inline ImgItem* GetExeItem() { return exeItem; }
  inline std::unordered_map<std::string, ImgItem*>& GetDllArray() {
    return dllArray;
  };
  inline std::queue<ImgItem*>& GetUnHandledDllQueue() {
    return unHandledDllQueue;
  }

  bool CreateImgArea(std::string path, bool isPE);
  DWORD DllLoader(std::string name);
  bool CallEntry();
  DWORD GetDllBase(const char* name);
};
#endif