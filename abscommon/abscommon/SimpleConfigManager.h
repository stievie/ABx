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

#include <lua.hpp>
#include <string>
#include <sa/Noncopyable.h>
#include <functional>
#include <sa/Iteration.h>
#include "Variant.h"

namespace IO {

class SimpleConfigManager
{
    NON_COPYABLE(SimpleConfigManager)
private:
    lua_State* L;
    bool loaded_;
protected:
    void RegisterString(const std::string& name, const std::string& value);
public:
    SimpleConfigManager() :
        L(nullptr),
        loaded_(false)
    { }
    ~SimpleConfigManager();
    std::string GetGlobalString(const std::string& ident, const std::string& def);
    int64_t GetGlobalInt(const std::string& ident, int64_t def);
    float GetGlobalFloat(const std::string& ident, float def);
    bool GetGlobalBool(const std::string& ident, bool def);
    bool GetTable(const std::string& ident,
        const std::function<Iteration(const std::string& name, const Utils::Variant& value)>& callback);

    bool Load(const std::string& file);
    void Close();
};

}
