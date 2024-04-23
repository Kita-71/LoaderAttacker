#include <iostream>

#include "FileReader.h"
#include "ImgManager.h"
#include "Terminal.h"
int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cerr
        << "Error: No arguments provided. Use --help for usage information.\n";
    return 1;
  }
  std::string arg1 = argv[1];
  ;
  std::string arg2 = (argv[2] == NULL) ? ("") : (argv[2]);

  if ((!arg1.compare("-h")) || (!arg1.compare("--help"))) {
    Terminal::ShowHelp();
  }
  if ((!arg1.compare("-l")) || (!arg1.compare("-load"))) {
    std::cout << "LOAD START" << std::endl;
    ImgManager imgManager;
    imgManager.CreateImgArea(arg2.c_str(), true);
    imgManager.Relocate(imgManager.GetPeItem());
    imgManager.FixImportTable(imgManager.GetPeItem());
    imgManager.CallEntry();
  }
  return 0;
}