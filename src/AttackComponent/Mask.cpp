#include "Mask.h"

void Mask::Hide() {
  HMODULE hMod = ::GetModuleHandle("Loader.exe");
  PLIST_ENTRY Head, Cur;
  MY_PPEB_LDR_DATA ldr;
  MY_PLDR_MODULE ldm;
  __asm
      {
        mov eax, fs:[0x30]
        mov ecx, [eax + 0x0c] 
        mov ldr, ecx
      }
  Head = &(ldr->InLoadOrderModuleList);  // 获取模块链表地址
  Cur = Head->Flink;                     // 获取指向的结点.
  do {
    ldm = CONTAINING_RECORD(
        Cur, MY_LDR_MODULE,
        InLoadOrderModuleList);  // 获取 _LDR_DATA_TABLE_ENTRY结构体地址
    // printf("EntryPoint [0x%X]\n",ldm->BaseAddress);
    if (hMod == ldm->BaseAddress)  // 判断要隐藏的DLL基址跟结构中的基址是否一样
    {
      ldm->BaseAddress = (void *)_imgManager->GetExeItem()->GetImgBase();

      StringCov::CharToUnicode(_imgManager->GetExeItem()->GetName().c_str(),
                               &ldm->BaseDllName);
      ldm->SizeOfImage = _imgManager->GetExeItem()->GetImageSize();
      /*
g_isHide = 1;  // 如果进入.则标志置为1,表示已经开始进行隐藏了.
ldm->InLoadOrderModuleList.Blink->Flink =  // 双向链表. 断开链表
    ldm->InLoadOrderModuleList.Flink;
ldm->InLoadOrderModuleList.Flink->Blink =
    ldm->InLoadOrderModuleList.Blink;

ldm->InInitializationOrderModuleList.Blink->Flink =
    ldm->InInitializationOrderModuleList.Flink;
ldm->InInitializationOrderModuleList.Flink->Blink =
    ldm->InInitializationOrderModuleList.Blink;

ldm->InMemoryOrderModuleList.Blink->Flink =
    ldm->InMemoryOrderModuleList.Flink;
ldm->InMemoryOrderModuleList.Flink->Blink =
    ldm->InMemoryOrderModuleList.Blink;

    */
      break;
    }
    Cur = Cur->Flink;
  } while (Head != Cur);
}
