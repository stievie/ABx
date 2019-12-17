#include "stdafx.h"
#include "QuestComp.h"
#include "Player.h"
#include "DataClient.h"
#include "Subsystems.h"

namespace Game {
namespace Components {

QuestComp::QuestComp(Player& owner) :
    owner_(owner)
{ }

void QuestComp::Update(uint32_t timeElapsed)
{
    for (const auto& q : quests_)
        q.second->Update(timeElapsed);
}

void QuestComp::Write(Net::NetworkMessage& message)
{
    for (const auto& q : quests_)
        q.second->Write(message);
}

bool QuestComp::GetReward(uint32_t questIndex)
{
    auto it = quests_.find(questIndex);
    if (it == quests_.end())
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
    // Remove from quest log
    quests_.erase(it);
    return true;
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
    const auto it = quests_.find(dq.index);
    if (it == quests_.end())
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
    auto it = quests_.find(index);
    if (it == quests_.end())
        return false;

    AB::Entities::PlayerQuest& pq = (*it).second->playerQuest_;
    pq.deleted = true;
    auto* client = GetSubsystem<IO::DataClient>();
    if (!client->Update(pq))
        return false;
    // Remove from quest log
    quests_.erase(it);
    return true;
}

bool QuestComp::HaveQuest(uint32_t index) const
{
    return quests_.find(index) != quests_.end();
}

bool QuestComp::Add(const AB::Entities::Quest& q, AB::Entities::PlayerQuest&& pq)
{
    std::unique_ptr<Quest> quest = std::make_unique<Quest>(owner_, std::move(pq));
    if (!quest->LoadScript(q.script))
    {
        LOG_ERROR << "Error loading quest script " << q.script << std::endl;
        return false;
    }
    quest->index_ = q.index;
    quests_.emplace(q.index, std::move(quest));
    return true;
}

}
}
