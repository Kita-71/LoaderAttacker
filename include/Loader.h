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
  ApiMsReader* apiMsReader_;

 public:
  Loader(ImgManager* imgManager, Relocator* relocator,
         ImpTableFixer* impTableFixer, ApiMsReader* apiMsReader)
      : imgManager_(imgManager),
        relocator_(relocator),
        impTableFixer_(impTableFixer),
        apiMsReader_(apiMsReader){};
  virtual ~Loader(){};
  void Load(std::string path);
  void Start();
};
#endif