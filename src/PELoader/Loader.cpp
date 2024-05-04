#include "Loader.h"
void Loader::Load(std::string path) {
  imgManager_->CreateImgArea(path, true);
  relocator_->Relocate(imgManager_->GetExeItem());
  impTableFixer_->FixImportTable(imgManager_, apiMsReader_,
                                 imgManager_->GetExeItem());
  while (!imgManager_->GetUnHandledDllQueue().empty()) {
    ImgItem* curItem = imgManager_->GetUnHandledDllQueue().front();
    relocator_->Relocate(curItem);
    impTableFixer_->FixImportTable(imgManager_, apiMsReader_, curItem);
    imgManager_->GetUnHandledDllQueue().pop();
  }
}
void Loader::Start() { imgManager_->CallEntry(); }