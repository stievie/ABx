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

#include "Game.h"
#include "GameObject.h"
#include <sa/Assert.h>
#include <algorithm>

void Game::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["Game"].setClass(kaguya::UserdataMetatable<Game>()
        .addFunction("GetObject", &Game::GetObject)
        .addFunction("GetObjectByName", &Game::GetObjectByName)
    );
    // clang-format on
}

Game::Game(AB::Entities::GameType type) :
    type_(type)
{
}

void Game::Update(uint32_t timeElapsed)
{
    for (const auto& object : objects_)
    {
        object.second->Update(timeElapsed);
    }
}

void Game::AddObject(std::unique_ptr<GameObject>&& object)
{
    ASSERT(object);
    uint32_t id = object->id_;
    object->SetGame(this);
    auto* obj = object.get();

    objects_.emplace(id, std::move(object));
    for (const auto& o : objects_)
        o.second->ObjectSpawn(obj);
}

void Game::RemoveObject(uint32_t id)
{
    auto* object = GetObject(id);
    if (!object)
        return;
    for (const auto& o : objects_)
        o.second->ObjectDespawn(object);

    object->SetGame(nullptr);
    auto it = objects_.find(id);
    if (it != objects_.end())
        objects_.erase(it);
}

GameObject* Game::GetObject(uint32_t id)
{
    const auto it = objects_.find(id);
    if (it != objects_.end())
        return (*it).second.get();
    return nullptr;
}

GameObject* Game::GetObjectByName(const std::string& name)
{
    const auto it = std::find_if(objects_.begin(), objects_.end(), [&name](const auto& current) {
        return current.second->name_.compare(name) == 0;
    });
    if (it != objects_.end())
        return (*it).second.get();
    return nullptr;
}
