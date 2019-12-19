#include "SqlReader.h"
#include <iostream>
#include <fstream>
#include <sa/StringTempl.h>

SqlReader::SqlReader()
{ }

bool SqlReader::Read(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in.is_open())
        return false;

    std::string line;
    while (std::getline(in, line))
    {
        line = sa::Trim(line);
        if (line.empty())
            continue;

        if (line.compare(0, 2, "--") == 0)
            continue;

        lines_.push_back(line);
    }
    return true;
}
