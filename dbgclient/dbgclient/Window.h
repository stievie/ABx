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

#include <sa/Compiler.h>
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
#include <panel.h>

// http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

struct Point
{
    int x;
    int y;
};

#define DEFAULT_BORDER_COLOR 1
#define ACTIVE_BORDER_COLOR 2
#define DEFAULT_LISTITEM_COLOR 3
#define SELECTED_LISTITEM_COLOR 4

class Window
{
public:
    enum Windows : size_t
    {
        WindowGames,
        WindowActors,
        WindowBehavior,
        __WindowsCount
    };
private:
    bool running_{ false };
    WINDOW* wins_[__WindowsCount];
    PANEL* panels_[__WindowsCount];
    PANEL* topPanel_{ nullptr };
    std::string statusText_;
    void ActivatePanel(PANEL* newTop);
    void CreateWindows();
    void DestroyWindows();
public:
    Window();
    ~Window();
    void Loop();
    void Print(const Point& pos, const std::string& text);
    void Goto(const Point& pos);
    void PrintStatusLine(const std::string& txt);
    void Clear();
    void BeginWindowUpdate(Windows window);
    void EndWindowUpdate(Windows window);
    void PrintGame(const std::string& txt, int index, bool selected);
    void PrintObject(const std::string& txt, int index, bool selected);
    void PrintObjectDetails(const std::string& txt, int line,
        bool selected = false, bool bold = false);
    void ShowCursor(bool visible);
    Point GetPosition() const;
    Point GetSize() const;
    bool IsRunning() const { return running_; }
    Windows GetActiveWindow() const;
    void SetStatusText(const std::string& value);

    std::function<void(Windows window, int c)> onKey_;
    std::function<void()> onResized_;
};

