#pragma once

namespace Utils {

std::string ConvertSize(size_t size);
uint32_t ConvertStringToIP(const std::string& ip);
std::string ConvertIPToString(uint32_t ip);

}
