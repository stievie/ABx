#pragma once

namespace Utils {

bool FileCopy(const std::string& src, const std::string& dst);
std::string NormalizeFilename(const std::string& filename);
bool FileExists(const std::string& name);
std::string AddSlash(const std::string& dir);
/// Files with a leading dot are hidden
bool IsHiddenFile(const std::string& path);

}