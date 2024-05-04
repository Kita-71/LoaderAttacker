#ifndef _API_MS_READER_
#define _API_MS_READER_
#include <Windows.h>

#include <string>
#include <unordered_map>
class ApiMsReader {
 private:
  std::unordered_map<std::string, std::string> apiMsMap;
  void GetApiSet();
  void ParseApiSet();
  DWORD apiSectionBase;
  DWORD apiMsDllBase;

 public:
  ApiMsReader();
  ~ApiMsReader(){};
  inline std::string CheckDll(std::string dllName) {
    std::string ret;
    if (apiMsMap.find(dllName) != apiMsMap.end()) ret = apiMsMap[dllName];
    return ret;
  };
};
typedef struct _API_SET_NAMESPACE {
  uint32_t Version;
  uint32_t Size;
  uint32_t Flags;
  uint32_t Count;
  uint32_t EntryOffset;
  uint32_t HashOffset;
  uint32_t HashFactor;
} API_SET_NAMESPACE, *PAPI_SET_NAMESPACE;

typedef struct _API_SET_HASH_ENTRY {
  uint32_t Hash;
  uint32_t Index;
} API_SET_HASH_ENTRY, *PAPI_SET_HASH_ENTRY;

typedef struct _API_SET_NAMESPACE_ENTRY {
  uint32_t Flags;
  uint32_t NameOffset;
  uint32_t NameLength;
  uint32_t HashedLength;
  uint32_t ValueOffset;
  uint32_t ValueCount;
} API_SET_NAMESPACE_ENTRY, *PAPI_SET_NAMESPACE_ENTRY;

typedef struct _API_SET_VALUE_ENTRY {
  uint32_t Flags;
  uint32_t NameOffset;
  uint32_t NameLength;
  uint32_t ValueOffset;
  uint32_t ValueLength;
} API_SET_VALUE_ENTRY, *PAPI_SET_VALUE_ENTRY;
#endif