#ifndef _MASK_H_
#define _MASK_H_
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "ImgManager.h"
#include "StringCov.h"

typedef struct _MY_PEB_LDR_DATA {
  ULONG Length;
  BOOLEAN Initialized;
  PVOID SsHandle;
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
} MY_PEB_LDR_DATA, *MY_PPEB_LDR_DATA;

typedef struct _MY_LDR_MODULE {
  LIST_ENTRY InLoadOrderModuleList;            //+0x00
  LIST_ENTRY InMemoryOrderModuleList;          //+0x08
  LIST_ENTRY InInitializationOrderModuleList;  //+0x10
  void* BaseAddress;                           //+0x18
  void* EntryPoint;                            //+0x1c
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG Flags;
  SHORT LoadCount;
  SHORT TlsIndex;
  HANDLE SectionHandle;
  ULONG CheckSum;
  ULONG TimeDateStamp;
} MY_LDR_MODULE, *MY_PLDR_MODULE;
class Mask {
 private:
  DWORD g_isHide = 0;
  ImgManager* _imgManager;

 public:
  Mask(ImgManager* imgManager) : _imgManager(imgManager){};
  virtual ~Mask(){};
  void Hide();
};
#endif