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

// http://tldp.org/HOWTO/NCURSES-Programming-HOWTO/

#include "Window.h"
#include <AB/CommonConfig.h>
#include <thread>

Window::Window()
{
    initscr();
    start_color();
    noecho();

    init_pair(DEFAULT_BORDER_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(ACTIVE_BORDER_COLOR, COLOR_YELLOW, COLOR_BLACK);
    init_pair(DEFAULT_LISTITEM_COLOR, COLOR_WHITE, COLOR_BLACK);
    init_pair(SELECTED_LISTITEM_COLOR, COLOR_BLACK, COLOR_WHITE);

    Point size = GetSize();
    int halfWidth = size.x / 2;

    wins_[0] = newwin(size.y - 1, halfWidth / 2, 0, 0);
    wins_[1] = newwin(size.y - 1, halfWidth / 2, 0, halfWidth / 2);
    wins_[2] = newwin(size.y - 1, halfWidth, 0, halfWidth);

    for (int i = 0; i < 3; ++i)
    {
        wattron(wins_[i], COLOR_PAIR(DEFAULT_BORDER_COLOR));
        box(wins_[i], 0, 0);
        wattroff(wins_[i], COLOR_PAIR(DEFAULT_BORDER_COLOR));
    }

    panels_[0] = new_panel(wins_[0]);
    panels_[1] = new_panel(wins_[1]);
    panels_[2] = new_panel(wins_[2]);

    /* Set up the user pointers to the next panel */
    set_panel_userptr(panels_[0], panels_[1]);
    set_panel_userptr(panels_[1], panels_[2]);
    set_panel_userptr(panels_[2], panels_[0]);

    ActivatePanel(panels_[0]);

    PrintStatusLine("q: Quit; Tab: Focus panel; Up/Down: Select");

    curs_set(FALSE);
}

Window::~Window()
{
    for (int i = 0; i < 3; ++i)
    {
        del_panel(panels_[i]);
        delwin(wins_[i]);
    }
    endwin();
}

char Window::GetChar() const
{
    int c = getch();
    return static_cast<char>(c);
}

void Window::ActivatePanel(PANEL* newTop)
{
    if (topPanel_ != nullptr)
    {
        wattron(topPanel_->win, COLOR_PAIR(DEFAULT_BORDER_COLOR));
        box(topPanel_->win, 0, 0);
        wattroff(topPanel_->win, COLOR_PAIR(DEFAULT_BORDER_COLOR));
    }
    topPanel_ = newTop;
    top_panel(topPanel_);

    wattron(topPanel_->win, COLOR_PAIR(ACTIVE_BORDER_COLOR));
    box(topPanel_->win, 0, 0);
    wattroff(topPanel_->win, COLOR_PAIR(ACTIVE_BORDER_COLOR));

    update_panels();
    doupdate();
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
    case 9:
        ActivatePanel((PANEL*)panel_userptr(topPanel_));
        break;
    }
}

void Window::Loop()
{
    running_ = true;
    while (running_)
    {
        char c = GetChar();

        HandleInput(c);
    }
}

void Window::PrintGame(const std::string& txt, int index)
{
    mvwprintw(wins_[0], index + 1, 1, txt.c_str());
    refresh();
}

void Window::PrintStatusLine(const std::string& txt)
{
    Point size = GetSize();
    Print({ 1, size.y - 1 }, txt);
}

void Window::Print(const Point& pos, const std::string& text)
{
    mvprintw(pos.y, pos.x, text.c_str());
    refresh();
}

void Window::Goto(const Point& pos)
{
    move(pos.y, pos.x);
}

Point Window::GetPosition() const
{
    Point result;
    getyx(stdscr, result.y, result.x);
    return result;
}

Point Window::GetSize() const
{
    Point result;
    getmaxyx(stdscr, result.y, result.x);
    return result;
}
void Window::Clear()
{
    clear();
}

void Window::ShowCursor(bool visible)
{
    curs_set(visible ? TRUE : FALSE);
}
