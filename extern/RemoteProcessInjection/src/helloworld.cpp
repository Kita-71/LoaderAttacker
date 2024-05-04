

#include <TlHelp32.h>
#include <Windows.h>

#include <iostream>
#include <vector>

using namespace std;

// 根据进程名称找到其PID
vector<DWORD> GetPIDByProcessName(LPCTSTR szProcessName) {
  STARTUPINFO st;
  PROCESS_INFORMATION pi;
  PROCESSENTRY32 ps;
  HANDLE hSnapshot;
  vector<DWORD> dwPID;

  ZeroMemory(&st, sizeof(STARTUPINFO));
  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
  st.cb = sizeof(STARTUPINFO);
  ZeroMemory(&ps, sizeof(PROCESSENTRY32));
  ps.dwSize = sizeof(PROCESSENTRY32);

  // 拍摄进程快照
  hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  // 快照拍摄失败
  if (hSnapshot == INVALID_HANDLE_VALUE) return dwPID;

  // 快照中是否含有进程
  if (!Process32First(hSnapshot, &ps)) return dwPID;

  do {
    // 遍历进程快照，比较进程名称
    if (lstrcmpi(ps.szExeFile, szProcessName) == 0) {
      // 将自身进程id放到容器中
      dwPID.push_back(ps.th32ProcessID);
    }
  } while (Process32Next(hSnapshot, &ps));

  // 关闭快照句柄
  CloseHandle(hSnapshot);

  return dwPID;
}

int main(int argc, char** argv) {
  cout << "搜寻目标进程...." << endl;

  // 目标进程的名称
  WCHAR targetName[] = L"notepad.exe";

  // 获取进程id
  vector<DWORD> hProcessId = GetPIDByProcessName((LPCSTR)targetName);
  if (hProcessId.size() == 0) {
    cout << "没有找到目标进程!" << endl;
    return 1;
  }

  cout << "-------------------------------------------------------" << endl;
  cout << "编号\t进程对应id(十六进制)\t进程对应id(十进制)" << endl;
  for (vector<DWORD>::size_type it = 0; it < hProcessId.size(); it++) {
    cout << it << "\t" << hex << hProcessId[it] << "\t\t\t" << dec
         << hProcessId[it] << endl;
  }

  // 目标dll的名称
  char dllName[] = "TestDll.dll";

  // 得到 kernel32 的模块句柄，因为 loadlibrary 在该dll中
  HMODULE kernel = GetModuleHandleA("kernel32.dll");

  // 此处应该使用 LoadLibraryA 或 LoadLibraryW，不能直接使用 LoadLibrary
  // 因为LoadLibrary 是一个宏，在代码运行时不能直接使用
  FARPROC loadlibrary = GetProcAddress(kernel, "LoadLibraryA");

  // 打开进程，获得进程句柄
  HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, hProcessId[0]);
  if (hProcess == NULL) {
    cout << "打开目标进程失败!" << endl;
    return 1;
  }

  // 在目标进程中分配空间，用于存储要加载的dll的名称
  LPVOID lpparameter = VirtualAllocEx(hProcess, NULL, sizeof(dllName),
                                      MEM_COMMIT, PAGE_EXECUTE_READWRITE);
  // 将目标dll的名字写入到目标进程
  WriteProcessMemory(hProcess, lpparameter, dllName, sizeof(dllName), NULL);

  // 创建远程线程
  HANDLE hThread = CreateRemoteThread(hProcess, NULL, NULL,
                                      (LPTHREAD_START_ROUTINE)loadlibrary,
                                      lpparameter, 0, NULL);
  // 等待远程线程运行结束
  WaitForSingleObject(hThread, INFINITE);

  CloseHandle(hThread);
  CloseHandle(hProcess);

  cout << "-------------------------------------------------------" << endl;
  cout << "成功注入进程：" << hProcessId[0] << endl;
  cout << "-------------------------------------------------------" << endl
       << endl;

  system("pause");

  return 0;
}