/**
 * Copyright 2020 Stefan Ascher
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

#include <string>
#include <map>
#include <sa/StringTempl.h>
#include <optional>

namespace IO {

class ConfigFile
{
private:
    std::map<std::string, std::string> entires_;
public:
    ConfigFile();
    ~ConfigFile();
    bool Load(const std::string& filename);
    template<typename T>
    inline T Get(const std::string& name, T def) const;

};

template<>
inline std::string ConfigFile::Get<std::string>(const std::string& name, std::string def) const
{
    const auto it = entires_.find(name);
    if (it == entires_.end())
        return def;
    return (*it).second;
}

template<>
inline bool ConfigFile::Get<bool>(const std::string& name, bool def) const
{
    const auto it = entires_.find(name);
    if (it == entires_.end())
        return def;
    return ((*it).second == "true" || (*it).second == "1" || (*it).second == "yes");
}

template<typename T>
inline T ConfigFile::Get(const std::string& name, T def) const
{
    const auto it = entires_.find(name);
    if (it == entires_.end())
        return def;
    const auto res = sa::to_number<T>((*it).second);
    if (res.has_value())
        return  res.value();
    return def;
}

}
