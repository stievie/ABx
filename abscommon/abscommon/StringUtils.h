#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace Utils {

/// Case insensitive string compare
bool StringEquals(const std::string& l, const std::string& r);
bool StringEquals(const std::wstring& l, const std::wstring& r);
std::string Utf8ToLower(const std::string& str);
std::wstring Utf8ToWString(const std::string& utf8);
std::string WStringToUtf8(const std::wstring& wstr);
// Same character name: UTF-8 case insensitive
bool SameName(const std::string& l, const std::string& r);
/// On Windows case insensitive, otherwise case sensitive
bool SameFilename(const std::string& l, const std::string& r);
std::string ChangeFileExt(const std::string& fn, const std::string& ext);
std::string GetFileExt(const std::string& fn);
std::string ExtractFileDir(const std::string& fn);
std::string ExtractFileName(const std::string& fn);

std::vector<std::string> Split(const std::string& str, const std::string& delim);
bool IsNumber(const std::string& s);
std::string ConvertSize(size_t size);
uint32_t ConvertStringToIP(const std::string& ip);
std::string ConvertIPToString(uint32_t ip, bool mask = false);

std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);

std::string EscapeArguments(const std::string& value);

}
