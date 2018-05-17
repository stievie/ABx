#include "stdafx.h"
#include "FileUtils.h"
#include <fstream>

namespace Utils {

bool FileExists(const std::string& name)
{
    std::ifstream infile(name.c_str());
    return infile.good();
}

}