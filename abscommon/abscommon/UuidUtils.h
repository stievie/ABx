#pragma once

#include <string>

namespace Utils {
namespace Uuid {

constexpr const char* EMPTY_UUID = "00000000-0000-0000-0000-000000000000";
bool IsEmpty(const std::string& uuid);
bool IsEqual(const std::string& u1, const std::string& u2);
std::string New();

}
}
