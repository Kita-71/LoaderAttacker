#ifndef _STRING_COV_H_
#define _STRING_COV_H_
#include <Windows.h>
#include <winternl.h>

#include <codecvt>
#include <locale>
#include <string>
#include <string.h>
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
  // UNICODE_STRINGz 转换为 CHAR*
  // 输入 UNICODE_STRING 的指针，输出窄字符串，BUFFER 需要已经分配好空间
  static void UnicodeToChar(PUNICODE_STRING dst, char* src) {
    ANSI_STRING string;
    RtlUnicodeStringToAnsiString(&string, (PCUNICODE_STRING)dst, TRUE);
    strcpy(src, string.Buffer);
    RtlFreeAnsiString(&string);
  }
};
#endif