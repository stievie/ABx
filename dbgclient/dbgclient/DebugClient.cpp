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
#include <iostream>

DebugClient::DebugClient(asio::io_service& io, Window& window) :
    client_(io),
    window_(window)
{
    client_.handlers_.Add<AI::GameAdd>(std::bind(&DebugClient::HandleGameAdd, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameRemove>(std::bind(&DebugClient::HandleGameRemove, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameSelected>(std::bind(&DebugClient::HandleGameSelected, this, std::placeholders::_1));
    client_.handlers_.Add<AI::ObjectUpdate>(std::bind(&DebugClient::HandleObjectUpdate, this, std::placeholders::_1));
}

bool DebugClient::Connect(const std::string& host, uint16_t port)
{
    bool r = client_.Connect(host, port);
    if (r)
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(100, std::bind(&DebugClient::GetGames, this)));
    return r;
}

void DebugClient::HandleGameAdd(const AI::GameAdd& message)
{
    games_[message.id] = message;
    UpdateGmes();
}

void DebugClient::HandleGameRemove(const AI::GameRemove& message)
{
    if (selectedGameIndex_ > -1)
    {
        if (static_cast<size_t>(selectedGameIndex_) >= gameIds_.size())
            selectedGameIndex_ = -1;
        else if (gameIds_[static_cast<size_t>(selectedGameIndex_)] == message.id)
            selectedGameIndex_ = -1;
    }
    games_.erase(message.id);
    UpdateGmes();
}

void DebugClient::HandleGameSelected(const AI::GameSelected &message)
{
    const auto it = std::find_if(gameIds_.begin(), gameIds_.end(), [&message](auto current) { return message.id == current; });
    if (it == gameIds_.end())
    {
        selectedGameIndex_ = -1;
        return;
    }
    auto index = std::distance(gameIds_.begin(), it);
    selectedGameIndex_ = static_cast<int>(index);
}

void DebugClient::UpdateGmes()
{
    int i = 0;
    gameIds_.clear();
    window_.BeginGameWindowUpdate();
    for (const auto& game : games_)
    {
        std::stringstream ss;
        ss << "[" << game.second.id << "] " << game.second.name;
        gameIds_.push_back(game.second.id);
        window_.PrintGame(ss.str(), i, selectedGameIndex_ == i);
        ++i;
    }
    window_.EndGameWindowUpdate();
}

void DebugClient::HandleObjectUpdate(const AI::ObjectUpdate& message)
{
    (void)message;
}

void DebugClient::GetGames()
{
    AI::GetGames msg;
    client_.Send(msg);
}

void DebugClient::SelectGame(uint32_t id)
{
    AI::SelectGame msg { id };
    client_.Send(msg);
}
