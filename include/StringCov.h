#ifndef _STRING_COV_H_
#define _STRING_COV_H_
#include <codecvt>
#include <locale>
#include <string>
class StringCov {
 public:
  // convert string to wstring
  static std::wstring toWideString(const std::string& input) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.from_bytes(input);
  }
  // convert wstring to string
  static std::string toByteString(const std::wstring& input) {
    // std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(input);
  }
};
#endif