#pragma once

#include <string_view>

namespace sa {

constexpr int char_pos(const char* str, const char n)
{
    return (*str && *str != n) ? 1 + char_pos(str + 1, n) : 0;
}

/// Compile time typeid(T).name()
/// constexpr auto res = sa::TypeName<Foo::Bar>::Get();
template<typename T>
struct TypeName
{
    static constexpr auto Get()
    {
#if defined __GNUC__
        constexpr const char* name = __PRETTY_FUNCTION__;
        constexpr int begin = char_pos(name, '=') + 2;
        constexpr int end = char_pos(name, ']');
#elif defined _MSC_VER
        constexpr const char* name = __FUNCTION__;
        constexpr int begin = char_pos(name, '<') + 1;
        constexpr int end = char_pos(name, '>');
#endif
        static_assert(end > begin);
        constexpr int length = end - begin;
        constexpr const char* ptr = &name[begin];
        return std::string_view(ptr, length);
    }
};

}
