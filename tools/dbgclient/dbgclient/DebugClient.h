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

#include <abipc/IpcClient.h>
#include <AB/IPC/AI/ServerMessages.h>
#include <map>
#include "Window.h"

class DebugClient
{
private:
    IPC::Client client_;
    Window& window_;
    std::map<uint32_t, AI::GameAdd> games_;
    std::map<uint32_t, AI::GameObject> objects_;
    std::map<uint32_t, AI::BehaviorTree> trees_;
    std::vector<uint32_t> gameIds_;
    size_t updatedObjectCount_{ 0 };
    size_t gameObjectCount_{ 0 };
    bool gameObjectsDirty_{ false };
    int selectedGameIndex_{ - 1 };
    uint32_t selectedObjectId_{ 0 };
    bool gamesDirty_{ false };
    void HandleGameAdd(const AI::GameAdd& message);
    void HandleGameRemove(const AI::GameRemove& message);
    void HandleGameUpdate(const AI::GameUpdate& message);
    void HandleGameObject(const AI::GameObject& message);
    void HandleGameSelected(const AI::GameSelected& message);
    void HandleTree(const AI::BehaviorTree& message);
    std::vector<std::pair<uint32_t, std::string>> PrintTreeStatus(const AI::BehaviorTree& tree,
        const std::vector<std::pair<uint32_t, int>>& status);
    void OnKey(Window::Windows window, int c);
    void UpdateGames();
    void UpdateObjects();
    void UpdateObjectDetails();
    void Initialize();
    void GetGames();
    void GetTrees();
    void SelectPrevObject();
    void SelectNextObject();
    void SelectGame(uint32_t id);
    uint32_t GetSelectedGameId() const;
public:
    DebugClient(asio::io_service& io, Window& window);
    bool Connect(const std::string& host, uint16_t port);
};

