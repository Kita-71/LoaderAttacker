#include "Relocator.h"
bool Relocator::Relocate(ImgItem* item) {
  // 获取重定位表的Virtual Address
  std::cout << "## RELOCATE START" << std::endl;
  DWORD relAddress = item->GetRelVirtualAddress();
  if (relAddress == 0) {
    std::cout << "## RELOCATE NOT EXIST,FINISH" << std::endl;
    return false;
  }
  PIMAGE_BASE_RELOCATION relTableBase =
      (PIMAGE_BASE_RELOCATION)(item->GetImgBase() + relAddress);
  PIMAGE_BASE_RELOCATION curRelBlock = relTableBase;
  // BUG点1，先判VA，再判SB
  while (curRelBlock->VirtualAddress != 0 && curRelBlock->SizeOfBlock != 0) {
    WORD* relBlockData =
        (WORD*)((DWORD)curRelBlock + sizeof(IMAGE_BASE_RELOCATION));
    int dataCount = (curRelBlock->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
                    sizeof(WORD);
    for (int i = 0; i < dataCount; i++) {
      if ((relBlockData[i] & 0xF000) == 0x3000)  // 标记该地址需要重定位
      {
        DWORD* pointerToData =
            (DWORD*)(item->GetImgBase() + curRelBlock->VirtualAddress +
                     (relBlockData[i] & 0x0FFF));
        DWORD imgExpectBase = item->GetFileExpectedBase();
        DWORD offset = (DWORD)item->GetImgBase() - imgExpectBase;
        *pointerToData += offset;
      }
    }
    curRelBlock =
        (PIMAGE_BASE_RELOCATION)((DWORD)curRelBlock + curRelBlock->SizeOfBlock);
  }
  std::cout << "## RELOCATE FINISH" << std::endl;
  return true;
};