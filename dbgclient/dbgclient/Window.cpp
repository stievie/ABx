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

#include "Window.h"
#include <AB/CommonConfig.h>
#include <thread>

Window::Window()
{
    initscr();
    start_color();
    noecho();
    curs_set(FALSE);
}

Window::~Window()
{
    endwin();
}

char Window::GetChar() const
{
    int c = getch();
    return static_cast<char>(c);
}

void Window::HandleInput(char c)
{
    if (onKey_)
        onKey_(c);

    switch (c)
    {
    case 'q':
        running_ = false;
        break;
    }
}

void Window::Loop()
{
    running_ = true;
    while (running_)
    {
        char c = GetChar();
        if (c == 0)
        {
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
            continue;
        }

        HandleInput(c);
    }
}

void Window::Print(const Point& pos, const std::string& text)
{
    pos_ = pos;
    mvprintw(pos.x, pos.y, text.c_str());
}

void Window::Goto(const Point& pos)
{
    pos_ = pos;
    move(pos.x, pos.y);
}

Point Window::GetPosition() const
{
    return pos_;
}

void Window::Clear()
{
    clear();
}

void Window::ShowCursor(bool visible)
{
    curs_set(visible ? TRUE : FALSE);
}
