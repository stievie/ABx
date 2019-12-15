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
