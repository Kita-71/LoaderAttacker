#include "Loader.h"
void Loader::Load(std::string path) {
  imgManager_->CreateImgArea(path, true);
  relocator_->Relocate(imgManager_->GetPeItem());
  impTableFixer_->FixImportTable(imgManager_, imgManager_->GetPeItem());
  while (!imgManager_->GetImgQueue().empty()) {
    ImgItem* curItem = imgManager_->GetImgQueue().front();
    relocator_->Relocate(curItem);
    impTableFixer_->FixImportTable(imgManager_, curItem);
    imgManager_->GetImgQueue().pop();
  }
  imgManager_->CallEntry();
}