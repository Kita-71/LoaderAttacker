#ifndef _LOADER_H_
#define _LOADER_H_
#include "ImgManager.h"
#include "ImpTableFixer.h"
#include "Relocator.h"

class Loader {
 private:
  ImgManager* imgManager_;
  Relocator* relocator_;
  ImpTableFixer* impTableFixer_;

 public:
  Loader(ImgManager* imgManager, Relocator* relocator,
         ImpTableFixer* impTableFixer)
      : imgManager_(imgManager),
        relocator_(relocator),
        impTableFixer_(impTableFixer){};
  virtual ~Loader(){};
  void Load(std::string path);
};
#endif