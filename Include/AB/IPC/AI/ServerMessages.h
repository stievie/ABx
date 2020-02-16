/**
 * Copyright 2017-2020 Stefan Ascher
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

#include <string>
#include <vector>
#include <array>
#include <stdint.h>
#include <map>

namespace AI {

struct GameAdd
{
    uint32_t id;
    std::string name;
    std::string mapUuid;
    std::string instanceUuid;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(name);
        ar.value(mapUuid);
        ar.value(instanceUuid);
    }
};

struct GameRemove
{
    uint32_t id;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
    }
};

struct GameSelected
{
    uint32_t id;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
    }
};

struct GameObject
{
    enum class ObjectType : uint8_t
    {
        Unknown,
        Npc,
        Player
    };

    uint32_t id{ 0 };
    uint32_t gameId{ 0 };
    ObjectType objectType{ ObjectType::Unknown };
    uint8_t objectState{ 0 };
    std::string name;
    std::array<float, 3> position;
    int health{ 0 };
    int maxHealth{ 0 };
    int energy{ 0 };
    int maxEnergy{ 0 };
    int morale{ 0 };
    int currentNodeStatus{ 0 };
    std::string currAction;
    uint32_t currActionId{ 0 };
    int selectedSkillIndex{ -1 };
    std::string selectedSkillName;
    size_t selectedAgentsCount{ 0 };
    std::vector<uint32_t> selectedAgents;
    size_t nodeStatusCount{ 0 };
    std::vector<std::pair<uint32_t, int>> nodeStatus;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(gameId);
        ar.value(objectType);
        ar.value(objectState);
        ar.value(name);
        ar.value(position[0]);
        ar.value(position[1]);
        ar.value(position[2]);
        ar.value(health);
        ar.value(maxHealth);
        ar.value(energy);
        ar.value(maxEnergy);
        ar.value(morale);
        ar.value(currentNodeStatus);
        ar.value(currAction);
        ar.value(currActionId);
        ar.value(selectedSkillIndex);
        ar.value(selectedSkillName);
        ar.value(selectedAgentsCount);
        selectedAgents.resize(selectedAgentsCount);
        for (size_t i = 0; i < selectedAgentsCount; ++i)
        {
            auto& oid = selectedAgents[i];
            ar.value(oid);
        }
        ar.value(nodeStatusCount);
        nodeStatus.resize(nodeStatusCount);
        for (size_t i = 0; i < nodeStatusCount; ++i)
        {
            auto& statusPair = nodeStatus[i];
            auto& statusId = statusPair.first;
            auto& statusStatus = statusPair.second;
            ar.value(statusId);
            ar.value(statusStatus);
        }
    }
};

struct GameUpdate
{
    uint32_t id;
    size_t count;
    std::vector<uint32_t> objects;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(count);
        objects.resize(count);
        for (size_t i = 0; i < count; ++i)
        {
            auto& oid = objects[i];
            ar.value(oid);
        }
    }
};

}
