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

#include "stdafx.h"
#include "QuestComp.h"
#include "Player.h"
#include "DataClient.h"
#include "Subsystems.h"
#include "Dispatcher.h"

namespace Game {
namespace Components {

QuestComp::QuestComp(Player& owner) :
    owner_(owner)
{ }

void QuestComp::Update(uint32_t timeElapsed)
{
    VisitActiveQuests([&] (Quest& current)
    {
        current.Update(timeElapsed);
        return Iteration::Continue;
    });
}

void QuestComp::Write(Net::NetworkMessage& message)
{
    VisitActiveQuests([&] (Quest& current)
    {
        current.Write(message);
        return Iteration::Continue;
    });
}

bool QuestComp::GetReward(uint32_t questIndex)
{
    auto it = activeQuests_.find(questIndex);
    if (it == activeQuests_.end())
        return false;
    Quest& quest = *(*it).second;
    if (!quest.IsCompleted())
        return false;
    if (!quest.CollectReward())
        return false;

    quest.SaveProgress();
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(quest.playerQuest_))
        return false;
    // Move to done quests
    doneQuests_.emplace(questIndex, std::move((*it).second));
    activeQuests_.erase(it);
    return true;
}

Quest* QuestComp::GetCompletedQuest(uint32_t index) const
{
    // We have to use a vector, because repeatable quests can be completed
    // multiple times (obviously)
    const auto it = doneQuests_.find(index);
    if (it == doneQuests_.end())
        return nullptr;
    return (*it).second.get();
}

Quest* QuestComp::Get(uint32_t index)
{
    const auto it = activeQuests_.find(index);
    if (it == activeQuests_.end())
        return GetCompletedQuest(index);
    return (*it).second.get();
}

const Quest* QuestComp::Get(uint32_t index) const
{
    const auto it = activeQuests_.find(index);
    if (it == activeQuests_.end())
        return GetCompletedQuest(index);
    return (*it).second.get();
}

bool QuestComp::IsAvailable(uint32_t index) const
{
    const auto* q = Get(index);
    if (!q)
        return true;
    if (q->IsActive())
        return false;
    if (q->IsRewarded() && !q->IsRepeatable())
        return false;
    return SatisfyRequirements(index);
}

bool QuestComp::IsActive(uint32_t index) const
{
    const auto* q = Get(index);
    if (!q)
        return false;
    return q->IsActive();
}

bool QuestComp::IsRewarded(uint32_t index) const
{
    const auto* q = Get(index);
    if (!q)
        return false;
    return q->playerQuest_.rewarded;
}

bool QuestComp::IsRepeatable(uint32_t index) const
{
    const auto* q = Get(index);
    if (!q)
        return false;
    return q->IsRepeatable();
}

bool QuestComp::SatisfyRequirements(const AB::Entities::Quest& q) const
{
    if (Utils::Uuid::IsEmpty(q.dependsOn))
        return true;

    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Quest dq;
    dq.uuid = q.dependsOn;
    if (!client->Read(dq))
    {
        LOG_ERROR << "Error reading quest" << std::endl;
        return false;
    }
    const auto it = activeQuests_.find(dq.index);
    if (it == activeQuests_.end())
        return false;
    return (*it).second->playerQuest_.rewarded;
}

bool QuestComp::SatisfyRequirements(uint32_t index) const
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Quest q;
    q.index = index;
    if (!client->Read(q))
    {
        LOG_ERROR << "Error reading quest" << std::endl;
        return false;
    }

    return SatisfyRequirements(q);
}

bool QuestComp::PickupQuest(uint32_t index)
{
    if (HaveQuest(index))
        return false;
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Quest q;
    q.index = index;
    if (!client->Read(q))
        return false;
    AB::Entities::PlayerQuest pq;
    pq.uuid = Utils::Uuid::New();
    pq.questUuid = q.uuid;
    pq.playerUuid = owner_.data_.uuid;
    if (!client->Create(pq))
        return false;
    return Add(q, std::move(pq));
}

bool QuestComp::DeleteQuest(uint32_t index)
{
    auto it = activeQuests_.find(index);
    if (it == activeQuests_.end())
        return false;

    return (*it).second->Delete();
}

bool QuestComp::HaveQuest(uint32_t index) const
{
    const auto it = activeQuests_.find(index);
    if (it == activeQuests_.end())
        return false;
    if ((*it).second->playerQuest_.deleted)
        return false;
    return true;
}

bool QuestComp::Add(const AB::Entities::Quest& q, AB::Entities::PlayerQuest&& pq)
{
    std::unique_ptr<Quest> quest = std::make_unique<Quest>(owner_, q, std::move(pq));
    if (!quest->LoadScript(q.script))
    {
        LOG_ERROR << "Error loading quest script " << q.script << std::endl;
        return false;
    }
    if (!pq.rewarded)
        activeQuests_.emplace(q.index, std::move(quest));
    else
        doneQuests_.emplace(q.index, std::move(quest));
    return true;
}

}
}
