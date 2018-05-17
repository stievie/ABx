#pragma once

#include <lua.hpp>

namespace IO {

class SimpleConfigManager
{
private:
    lua_State* L;
    bool isLoaded;
public:
    SimpleConfigManager() :
        L(nullptr),
        isLoaded(false)
    { }
    ~SimpleConfigManager()
    {
        if (L)
            lua_close(L);
    }
    std::string GetGlobal(const std::string& ident, const std::string& default);
    int64_t GetGlobal(const std::string& ident, int64_t default);
    bool GetGlobalBool(const std::string& ident, bool default);

    bool Load(const std::string& file);
    static SimpleConfigManager Instance;
};

}
