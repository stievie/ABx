#pragma once

#include "Asset.h"

namespace Game {

template<typename CharT, typename TraitsT = std::char_traits<CharT> >
class vectorwrapbuf : public std::basic_streambuf<CharT, TraitsT>
{
public:
    vectorwrapbuf(std::vector<CharT> &vec)
    {
        setg(vec.data(), vec.data(), vec.data() + vec.size());
    }
};

class LuaScript : public IO::AssetImpl<LuaScript>
{
private:
    std::vector<char> buffer_;
public:
    LuaScript() = default;
    ~LuaScript() override
    { }

    std::vector<char>& GetBuffer()
    {
        return buffer_;
    }

    bool Execute(kaguya::State& luaState);
};

}
