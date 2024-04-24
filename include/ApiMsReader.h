#ifndef _API_MS_READER_
#define _API_MS_READER_
#include <string>
class ApiMsReader {
 private:
 public:
  ApiMsReader();
  ~ApiMsReader();
  std::string CheckDll(std::string dllName);
};
#endif