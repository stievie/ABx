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

#include "DebugClient.h"
#include <AB/IPC/AI/ClientMessages.h>
#include "Scheduler.h"
#include "Subsystems.h"

DebugClient::DebugClient(asio::io_service& io, Window& window) :
    client_(io),
    window_(window)
{
    window_.onKey_ = [this](char& c)
    {
        return HandleOnKey(c);
    };
    client_.handlers_.Add<AI::GameAdd>(std::bind(&DebugClient::HandleGameAdd, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameRemove>(std::bind(&DebugClient::HandleGameRemove, this, std::placeholders::_1));
    client_.handlers_.Add<AI::ObjectUpdate>(std::bind(&DebugClient::HandleObjectUpdate, this, std::placeholders::_1));
}

void DebugClient::HandleOnKey(char& c)
{
    switch (screen_)
    {
    case Screen::SelectGame:
        return HandleSelectGameOnKey(c);
    case Screen::Game:
        return HandleGameOnKey(c);
    }
}

void DebugClient::HandleSelectGameOnKey(char& c)
{
    if (isdigit(c))
    {
        int index = c - 48;
        if (index >= 0 && index < gameIds_.size())
        {
            SelectGame(gameIds_[index]);
        }
    }
}

void DebugClient::HandleGameOnKey(char& c)
{
    switch (c)
    {
    case 8:
        // Backspace
        screen_ = Screen::SelectGame;
        gamesDirty_ = true;
        break;
    }
    (void)c;
}

bool DebugClient::Connect(const std::string& host, uint16_t port)
{
    bool r = client_.Connect(host, port);
    if (r)
    {
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(1000, std::bind(&DebugClient::GetGames, this)));
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(1000, std::bind(&DebugClient::UpdateScreen, this)));
    }
    gamesDirty_ = true;
    return r;
}

void DebugClient::HandleGameAdd(const AI::GameAdd& message)
{
    games_[message.id] = message;
    gamesDirty_ = true;
}

void DebugClient::HandleGameRemove(const AI::GameRemove& message)
{
    games_.erase(message.id);
    gamesDirty_ = true;
}

void DebugClient::HandleObjectUpdate(const AI::ObjectUpdate& message)
{
    if (screen_ == Screen::SelectGame)
    {
        screen_ = Screen::Game;
        window_.Clear();
    }


    (void)message;
}

void DebugClient::UpdateScreen()
{
    switch (screen_)
    {
    case Screen::SelectGame:
        DrawSelectGame();
        break;
    case Screen::Game:
        DrawGame();
        break;
    }
    if (window_.IsRunning())
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(100, std::bind(&DebugClient::UpdateScreen, this)));
}

void DebugClient::DrawSelectGame()
{
    if (!gamesDirty_)
        return;

    gamesDirty_ = false;
    window_.Clear();
    if (games_.size() == 0)
    {
        window_.Print({ 0, 0 }, "It seems there are no games running");
        return;
    }

    window_.SetColor(Window::FG_BLACK, Window::BG_WHITE);
    window_.Print({ 0, 0 }, "Select game");
    window_.RestColor();
    Point p = { 0, 2 };
    int i = 0;
    gameIds_.clear();
    for (const auto& game : games_)
    {
        std::stringstream ss;
        ss << "[" << i << "] " << game.second.name << " " << game.second.instanceUuid;
        gameIds_.push_back(game.second.id);
        ++i;
        window_.Print({ p.x, p.y }, ss.str());
        ++p.y;
    }
}

void DebugClient::DrawGame()
{
}

void DebugClient::GetGames()
{
    AI::GetGames msg;
    client_.Send(msg);
}

void DebugClient::SelectGame(uint32_t id)
{
    AI::SelectGame msg;
    msg.gameId = id;
    client_.Send(msg);
}
