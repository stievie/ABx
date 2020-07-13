/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <string>
#include <vector>

namespace Utils {

static const std::string EMPTY_STRING = "";

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
std::string ConcatPath(const std::string& path, const std::string& name, const std::string& ext = EMPTY_STRING);

bool IsNumber(const std::string& s);
std::string ConvertSize(size_t size);
uint32_t ConvertStringToIP(const std::string& ip);
std::string ConvertIPToString(uint32_t ip, bool mask = false);

std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);

std::string EscapeArguments(const std::string& value);

}
