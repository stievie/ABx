#pragma once
#include <ostream>

// https://misc.flogisoft.com/bash/tip_colors_and_formatting
namespace IO {
namespace Color {
    enum Code
    {
        // Foregrounds
        FG_BLACK        = 30,
        FG_RED          = 31,
        FG_GREEN        = 32,
        FG_YELLOW       = 33,
        FG_BLUE         = 34,
        FG_MAGENTA      = 35,
        FG_CYAN         = 36,
        FG_LIGHTGREY    = 37,
        FG_DEFAULT      = 39,
        FG_DARKGREY     = 90,
        FG_LIGHRED      = 91,
        FG_LIGHTGREEN   = 92,
        FG_LIGHTYELLOW  = 93,
        FG_LIGHTBLUE    = 94,
        FG_LIGHTMAGENTA = 95,
        FG_LIGHTCYAN    = 96,
        FG_WHITE        = 97,
        // Backgrounds
        BG_BLACK        = 40,
        BG_RED          = 41,
        BG_GREEN        = 42,
        BG_YELLOW       = 43,
        BG_BLUE         = 44,
        BG_MAGENTA      = 45,
        BG_CYAN         = 46,
        BG_LIGHTGREY    = 47,
        BG_DEFAULT      = 49,
        BG_DARKGREY     = 100,
        BG_LIGHTRED     = 101,
        BG_LIGHTGREEN   = 102,
        BG_LIGHTYELLOW  = 103,
        BG_LIGHTBLUE    = 104,
        BG_LIGHTMAGENTA = 105,
        BG_LIGHTCYAN    = 106,
        BG_WHITE        = 107,
    };
    class Modifier
    {
    private:
        Code code_;
    public:
        explicit Modifier(Code code) : code_(code) {}
        friend std::ostream&
        operator << (std::ostream& os, const Modifier& mod)
        {
            return os << "\033[" << mod.code_ << "m";
        }
    };
}
}
