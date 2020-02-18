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
#include <AB/ProtocolCodes.h>
#include <abai/Node.h>
#include <abscommon/Scheduler.h>
#include <abscommon/Subsystems.h>
#include <iostream>
#include <sa/StringTempl.h>

static std::string GetObjectStateString(uint8_t state)
{
    switch (static_cast<AB::GameProtocol::CreatureState>(state))
    {
#define ENUMERATE_CREATURE_STATE(v)          \
    case AB::GameProtocol::CreatureState::v: \
        return #v;
        ENUMERATE_CREATURE_STATES
#undef ENUMERATE_CREATURE_STATE
    }
    return "???";
}

static std::string GetNodeStatusString(AI::Node::Status status)
{
    switch (status)
    {
    case AI::Node::Status::Unknown:
        return "Unknown";
    case AI::Node::Status::CanNotExecute:
        return "CanNotExecute";
    case AI::Node::Status::Running:
        return "Running";
    case AI::Node::Status::Finished:
        return "Finished";
    case AI::Node::Status::Failed:
        return "Failed";
    default:
        return "???";
    }
}

DebugClient::DebugClient(asio::io_service& io, Window& window) :
    client_(io),
    window_(window)
{
    window_.onKey_ = std::bind(&DebugClient::OnKey, this, std::placeholders::_1, std::placeholders::_2);
    client_.handlers_.Add<AI::GameAdd>(std::bind(&DebugClient::HandleGameAdd, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameRemove>(std::bind(&DebugClient::HandleGameRemove, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameSelected>(std::bind(&DebugClient::HandleGameSelected, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameUpdate>(std::bind(&DebugClient::HandleGameUpdate, this, std::placeholders::_1));
    client_.handlers_.Add<AI::GameObject>(std::bind(&DebugClient::HandleGameObject, this, std::placeholders::_1));
    client_.handlers_.Add<AI::BehaviorTree>(std::bind(&DebugClient::HandleTree, this, std::placeholders::_1));
}

bool DebugClient::Connect(const std::string& host, uint16_t port)
{
    std::string status = "q: Quit; Tab: Focus window; Up/Down: Select";
    bool r = client_.Connect(host, port);
    if (r)
    {
        std::stringstream ss;
        ss << "; Connected to: " << host << ":" << port;
        status += ss.str();
        GetSubsystem<Asynch::Scheduler>()->Add(Asynch::CreateScheduledTask(100, std::bind(&DebugClient::Initialize, this)));
    }
    else
        status += "; Not connected";
    window_.SetStatusText(status);
    return r;
}

void DebugClient::HandleGameAdd(const AI::GameAdd& message)
{
    games_[message.id] = message;
    UpdateGames();
}

void DebugClient::HandleGameRemove(const AI::GameRemove& message)
{
    if (selectedGameIndex_ > -1)
    {
        if (static_cast<size_t>(selectedGameIndex_) >= gameIds_.size())
        {
            selectedGameIndex_ = -1;
            selectedObjectId_ = 0;
        }
        else if (gameIds_[static_cast<size_t>(selectedGameIndex_)] == message.id)
        {
            selectedGameIndex_ = -1;
            selectedObjectId_ = 0;
        }
    }
    games_.erase(message.id);
    UpdateGames();
}

void DebugClient::HandleGameSelected(const AI::GameSelected& message)
{
    const auto it = std::find_if(gameIds_.begin(), gameIds_.end(), [&message](auto current) { return message.id == current; });
    if (it == gameIds_.end())
    {
        selectedGameIndex_ = -1;
        selectedObjectId_ = 0;
        return;
    }
    selectedObjectId_ = 0;
    auto index = std::distance(gameIds_.begin(), it);
    selectedGameIndex_ = static_cast<int>(index);
    UpdateGames();
    updatedObjectCount_ = 0;

    window_.BeginWindowUpdate(Window::WindowActors);
    window_.EndWindowUpdate(Window::WindowActors);
}

void DebugClient::HandleTree(const AI::BehaviorTree& message)
{
    trees_.emplace(message.id, message);
}

void DebugClient::UpdateGames()
{
    int i = 0;
    gameIds_.clear();
    window_.BeginWindowUpdate(Window::WindowGames);
    for (const auto& game : games_)
    {
        std::stringstream ss;
        ss << "[" << game.second.id << "] " << game.second.name;
        gameIds_.push_back(game.second.id);
        window_.PrintGame(ss.str(), i, selectedGameIndex_ == i);
        ++i;
    }
    window_.EndWindowUpdate(Window::WindowGames);
}

void DebugClient::UpdateObjects()
{
    updatedObjectCount_ = 0;

    int i = 0;
    window_.BeginWindowUpdate(Window::WindowActors);
    for (const auto& object : objects_)
    {
        std::stringstream ss;
        ss << "[" << object.second.id << "] " << object.second.name;
        window_.PrintObject(ss.str(), i, object.second.id == selectedObjectId_);
        ++i;
    }
    window_.EndWindowUpdate(Window::WindowActors);
}

void DebugClient::UpdateObjectDetails()
{
    if (selectedObjectId_ == 0)
        return;

    const auto it = objects_.find(selectedObjectId_);
    if (it == objects_.end())
        return;

    window_.BeginWindowUpdate(Window::WindowBehavior);

    int line = 0;
    const AI::GameObject& obj = objects_[selectedObjectId_];
    {
        std::stringstream ss;
        ss << "[" << obj.id << "] " << obj.name;
        window_.PrintObjectDetails(ss.str(), line++, false, true);
    }
    {
        std::stringstream ss;
        ss << "Position: " << obj.position[0] << "," << obj.position[1] << "," << obj.position[2];
        window_.PrintObjectDetails(ss.str(), line++);
    }

    {
        std::stringstream ss;
        ss << "State: " << GetObjectStateString(obj.objectState);
        window_.PrintObjectDetails(ss.str(), line++);
    }
    {
        std::stringstream ss;
        ss << "HP: " << obj.health << "/" << obj.maxHealth << "; E: " << obj.energy << "/" << obj.maxEnergy << "; M: " << obj.morale;
        window_.PrintObjectDetails(ss.str(), line++);
    }
    line++;

    window_.PrintObjectDetails("AI", line++, false, true);
    {
        std::stringstream ss;
        ss << "Action: "
           << "[" << obj.currActionId << "] " << obj.currAction;
        window_.PrintObjectDetails(ss.str(), line++);
    }
    {
        std::stringstream ss;
        ss << "Node status: " << GetNodeStatusString(static_cast<AI::Node::Status>(obj.currentNodeStatus));
        window_.PrintObjectDetails(ss.str(), line++);
    }
    {
        std::stringstream ss;
        ss << "Skill: "
           << "[" << obj.selectedSkillIndex << "] " << obj.selectedSkillName;
        window_.PrintObjectDetails(ss.str(), line++);
    }
    {
        std::stringstream ss;
        ss << "Selection: ";
        for (uint32_t a : obj.selectedAgents)
            ss << a << ",";
        std::string str = sa::Trim(ss.str(), std::string(","));
        window_.PrintObjectDetails(str, line++);
    }

    const auto treeit = trees_.find(obj.behaviorId);
    if (treeit != trees_.end())
    {
        const auto& tree = (*treeit).second;
        std::vector<std::string> treeStatus = PrintTreeStatus(tree, obj.nodeStatus);

        for (const auto& s : treeStatus)
        {
            window_.PrintObjectDetails(s, line++);
        }
    }
    else
    {
        std::stringstream ss;
        ss << "[" << obj.behaviorId << "]: UNKNOWN ROOT NODE";
        window_.PrintObjectDetails(ss.str(), line++);
        for (const auto& ns : obj.nodeStatus)
        {
            std::stringstream ss2;
            ss2 << "  [" << ns.first << "] " << GetNodeStatusString(static_cast<AI::Node::Status>(ns.second));
            window_.PrintObjectDetails(ss2.str(), line++, false, ns.first == obj.currActionId);
        }
    }

    window_.EndWindowUpdate(Window::WindowBehavior);
}

std::vector<std::string> DebugClient::PrintTreeStatus(const AI::BehaviorTree& tree,
    const std::vector<std::pair<uint32_t, int>>& status)
{
    std::vector<std::string> result;

    std::map<uint32_t, int> indent;

    indent.emplace(tree.id, 0);

    for (const auto& tc : tree.nodes)
    {
        int in = indent[tc.parentId];
        indent.emplace(tc.id, in + 1);
    }
    {
        std::stringstream ss;
        ss << "[" << tree.id << "] " << tree.name;
        result.push_back(ss.str());
    }
    for (const auto& tc : tree.nodes)
    {
        int in = indent[tc.id];
        std::stringstream ss;
        char s[32] = {};
        sprintf(s, "%*c", in, ' ');
        ss << s;
        ss << "[" << tc.id << "] " << tc.name;
        const auto it = std::find_if(status.begin(), status.end(), [&](const std::pair<uint32_t, int>& current)
        {
            return current.first == tc.id;
        });
        if (it != status.end())
            ss << ": " << GetNodeStatusString(static_cast<AI::Node::Status>((*it).second));
        result.push_back(ss.str());
    }
    return result;
}

void DebugClient::Initialize()
{
    GetTrees();
    GetGames();
}

void DebugClient::HandleGameUpdate(const AI::GameUpdate& message)
{
    objects_.clear();
    updatedObjectCount_ = 0;
    for (const auto& o : message.objects)
    {
        objects_.emplace(o, AI::GameObject());
    }
}

void DebugClient::HandleGameObject(const AI::GameObject& message)
{
    if (message.gameId != GetSelectedGameId())
        return;
    const auto it = objects_.find(message.id);
    if (it == objects_.end())
        return;

    objects_[message.id] = message;
    ++updatedObjectCount_;

    if (updatedObjectCount_ == objects_.size())
    {
        UpdateObjects();
        UpdateObjectDetails();
    }
}

void DebugClient::OnKey(Window::Windows window, int c)
{
    switch (c)
    {
    case KEY_UP:
        switch (window)
        {
        case Window::Windows::WindowGames:
            if (selectedGameIndex_ > 0)
            {
                SelectGame(gameIds_[static_cast<size_t>(selectedGameIndex_ - 1)]);
            }
            break;
        case Window::Windows::WindowActors:
            SelectPrevObject();
            break;
        case Window::Windows::WindowBehavior:
        default:
            break;
        }
        break;
    case KEY_DOWN:
        switch (window)
        {
        case Window::Windows::WindowGames:
        {
            if (selectedGameIndex_ >= static_cast<int>(games_.size()) - 1)
                return;
            int i = selectedGameIndex_;
            if (i == -1)
                i = 0;
            else if (i < static_cast<int>(games_.size()) - 1)
                ++i;
            SelectGame(gameIds_[static_cast<size_t>(i)]);
            break;
        }
        case Window::Windows::WindowActors:
            SelectNextObject();
            break;
        case Window::Windows::WindowBehavior:
            break;
        default:
            break;
        }
        break;
    }
}

void DebugClient::GetGames()
{
    AI::GetGames msg;
    client_.Send(msg);
}

void DebugClient::GetTrees()
{
    AI::GetTrees msg;
    client_.Send(msg);
}

void DebugClient::SelectPrevObject()
{
    if (selectedObjectId_ == 0)
        return;
    auto it = objects_.find(selectedObjectId_);
    if (it == objects_.end())
    {
        selectedObjectId_ = 0;
        return;
    }
    if (it == objects_.begin())
    {
        return;
    }
    it--;
    selectedObjectId_ = (*it).second.id;
}

void DebugClient::SelectNextObject()
{
    if (selectedObjectId_ == 0)
    {
        if (objects_.size() == 0 || (*objects_.begin()).second.id == 0)
            return;
        selectedObjectId_ = (*objects_.begin()).second.id;
        return;
    }
    auto it = objects_.find(selectedObjectId_);
    if (it == objects_.end())
    {
        selectedObjectId_ = 0;
        return;
    }
    it++;
    if (it == objects_.end())
        return;
    selectedObjectId_ = (*it).second.id;
}

void DebugClient::SelectGame(uint32_t id)
{
    AI::SelectGame msg{ id };
    client_.Send(msg);
}

uint32_t DebugClient::GetSelectedGameId() const
{
    if (selectedGameIndex_ < 0 || selectedGameIndex_ >= static_cast<int>(gameIds_.size()))
        return 0;
    return gameIds_[static_cast<size_t>(selectedGameIndex_)];
}
