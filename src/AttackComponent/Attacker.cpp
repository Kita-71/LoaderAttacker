#include "Attacker.h"

#include <windows.h>
template <typename AddressType, typename FuncPtrType>
AddressType union_cast(FuncPtrType func_ptr)  // 获取类内成员函数的函数地址
{
  union {
    FuncPtrType f;
    AddressType d;
  } u;
  u.f = func_ptr;
  return u.d;
}

#include "ExpTableReader.h"
void Attacker::Attack() {
  InstallHook();
  return;
}
// Hook函数实现
void HookedHello() {
  Attacker::getInstance(NULL).UnpatchFunction(
      (BYTE*)Attacker::getInstance(NULL).originalHello);
  MessageBoxA(0, "You are Hacked!", "Warning", MB_OK);
  // 调用原始的hello函数（如果需要）
  if (Attacker::getInstance(NULL).originalHello) {
    Attacker::getInstance(NULL).originalHello();
  }
  Attacker::getInstance(NULL).InstallHook();
}

void Attacker::InstallHook() {
  // 这里我们需要获取originalHello的实际地址，并将其保存
  DWORD base = _imgManager->GetDllBase("testDll.dll");
  originalHello = (pHello)ExpTableReader::GetFuncAddr(
      _imgManager->GetDllArray()["testDll.dll"], 1, "?hello@@YAXXZ");

  // 这里使用一个简单的JMP指令直接覆盖原函数的起始部分
  void* pJmpToHookedHello = &HookedHello;

  PatchFunction((BYTE*)originalHello, pJmpToHookedHello);
}
void Attacker::PatchFunction(BYTE* dst, void* hook) {
  DWORD oldProtect, oldProtect2, jumpOffset;
  // 首先保存原始字节
  memcpy(originalBytes, dst, sizeof(originalBytes));
  // 计算跳转偏移量
  jumpOffset = ((DWORD)hook - (DWORD)dst - 5);
  // 开启内存页的写权限，以便我们可以修改代码
  VirtualProtect(dst, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
  // 写入JMP指令和跳转偏移量
  dst[0] = 0xE9;
  *(DWORD*)(dst + 1) = jumpOffset;
  // 恢复原始内存页权限
  VirtualProtect(dst, 5, oldProtect, &oldProtect2);
}

// 使用保存的原始字节恢复函数
void Attacker::UnpatchFunction(BYTE* dst) {
  DWORD oldProtect;
  // 开启内存页的写权限
  VirtualProtect(dst, 5, PAGE_EXECUTE_READWRITE, &oldProtect);
  // 恢复原始字节
  memcpy(dst, originalBytes, sizeof(originalBytes));
  // 恢复原始内存页权限
  VirtualProtect(dst, 5, oldProtect, &oldProtect);
  // 清除指令缓存
  FlushInstructionCache(GetCurrentProcess(), dst, 5);
}