#include "Platform.h"
#include <AB/CommonConfig.h>
#include <fstream>

namespace System {

std::string GetPlatform()
{
#ifdef AB_WINDOWS
    return "Windows";
#else
    std::ifstream in("/etc/os-release");
    std::string line;
    while (std::getline(in, line))
    {
        if (line.find("ID=") != std::string::npos)
        {
            return line.substr(3);
        }
    }
    return "Linux";
#endif
}

}