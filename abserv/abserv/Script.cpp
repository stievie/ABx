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

#include "stdafx.h"
#include "Script.h"

namespace Game {

namespace Internal {
template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vectorbuf : public std::basic_streambuf<CharT, TraitsT>
{
public:
    vectorbuf(ea::vector<CharT>& vec)
    {
        this->setg(vec.data(), vec.data(), vec.data() + vec.size());
    }
};
}

Script::~Script() = default;

bool Script::Execute(kaguya::State& luaState)
{
    Internal::vectorbuf<char> databuf(buffer_);
    std::istream is(&databuf);
    if (!luaState.dostream(is))
    {
        LOG_ERROR << lua_tostring(luaState.state(), -1) << std::endl;
        return false;
    }
    return true;
}

}
