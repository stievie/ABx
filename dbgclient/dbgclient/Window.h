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

#include <sa/PragmaWarning.h>
#include <string>
#include <functional>
#include <AB/CommonConfig.h>
#if defined(AB_UNIX)
#include <ncurses.h>
#elif defined(AB_WINDOWS)
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4005)
#include <curses.h>
PRAGMA_WARNING_POP
#endif

struct Point
{
    int x;
    int y;
};

class Window
{
private:
    bool running_{ false };
    Point pos_{ 0, 0 };

    void HandleInput(char c);
public:
    Window();
    ~Window();
    char GetChar() const;
    void Loop();
    void Print(const Point& pos, const std::string& text);
    void Goto(const Point& pos);
    void Clear();
    void ShowCursor(bool visible);
    Point GetPosition() const;
    bool IsRunning() const { return running_; }

    std::function<void(char& c)> onKey_;
};

