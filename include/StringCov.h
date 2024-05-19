#ifndef _STRING_COV_H_
#define _STRING_COV_H_
#include <Windows.h>
#include <string.h>
#include <winternl.h>

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
  // UNICODE_STRINGz 转换为 CHAR*
  // 输入 UNICODE_STRING 的指针，输出窄字符串，BUFFER 需要已经分配好空间
  static void UnicodeToChar(PUNICODE_STRING dst, char* src) {
    ANSI_STRING string;
    RtlUnicodeStringToAnsiString(&string, (PCUNICODE_STRING)dst, TRUE);
    strcpy(src, string.Buffer);
    RtlFreeAnsiString(&string);
  }

  // 将 const char* 转换为自定义的 UNICODE_STRING 结构
  static void CharToUnicode(const char* src, PUNICODE_STRING dst) {
    // 首先，使用 MultiByteToWideChar 计算需要的宽字符数
    int wcharCount = MultiByteToWideChar(CP_UTF8, 0, src, -1, NULL, 0);

    // 分配宽字符缓冲区
    dst->Buffer = (PWSTR)malloc(wcharCount * sizeof(WCHAR));
    if (dst->Buffer) {
      // 执行字符串转换
      MultiByteToWideChar(CP_UTF8, 0, src, -1, dst->Buffer, wcharCount);

      // 设置 UNICODE_STRING 结构的其他成员
      dst->Length =
          (USHORT)((wcharCount - 1) * sizeof(WCHAR));  // 不包含null终止符
      dst->MaximumLength = (USHORT)(wcharCount * sizeof(WCHAR));
    } else {
      // 如果内存分配失败，则清除字段
      dst->Length = 0;
      dst->MaximumLength = 0;
      dst->Buffer = NULL;
    }
  }
};
#endif