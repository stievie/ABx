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
#include "IOPlayer.h"
#include "ConfigManager.h"
#include "IOGame.h"
#include "SkillManager.h"
#include "QuestComp.h"
#include "Player.h"
#include <AB/Entities/Account.h>
#include <AB/Entities/AccountItemList.h>
#include <AB/Entities/FriendList.h>
#include <AB/Entities/FriendedMe.h>
#include <AB/Entities/GuildMembers.h>
#include <AB/Entities/PlayerItemList.h>
#include <AB/Entities/PlayerQuest.h>
#include <AB/Entities/PlayerQuestList.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Quest.h>
#include <uuid.h>

namespace IO {
namespace IOPlayer {

static bool LoadPlayerInventory(Game::Player& player)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    // Equipment
    AB::Entities::EquippedItems equipmenet;
    equipmenet.uuid = player.data_.uuid;
    if (client->Read(equipmenet))
    {
        for (const auto& e : equipmenet.itemUuids)
            player.SetEquipment(e);
    }

    // Inventory
    AB::Entities::InventoryItems inventory;
    inventory.uuid = player.data_.uuid;
    if (client->Read(inventory))
    {
        for (const auto& e : inventory.itemUuids)
            player.SetInventory(e);
    }

    // Chest
    AB::Entities::ChestItems chest;
    chest.uuid = player.account_.uuid;
    if (client->Read(chest))
    {
        for (const auto& e : chest.itemUuids)
            player.SetChest(e);
    }

    return true;
}

static bool SavePlayerInventory(Game::Player& player)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    // Equipment
    player.inventoryComp_->VisitEquipement([client](Game::Item& item)
    {
        client->Update(item.concreteItem_);
        client->Invalidate(item.concreteItem_);
        return Iteration::Continue;
    });

    // Inventory
    player.inventoryComp_->VisitInventory([client](Game::Item& item)
    {
        client->Update(item.concreteItem_);
        client->Invalidate(item.concreteItem_);
        return Iteration::Continue;
    });
    AB::Entities::InventoryItems inventory;
    inventory.uuid = player.data_.uuid;
    client->Invalidate(inventory);

    // Chest
    player.inventoryComp_->VisitChest([client](Game::Item& item)
    {
        client->Update(item.concreteItem_);
        client->Invalidate(item.concreteItem_);
        return Iteration::Continue;
    });

    AB::Entities::ChestItems chest;
    chest.uuid = player.account_.uuid;
    client->Invalidate(chest);
    return true;
}

static bool LoadQuestLog(Game::Player& player)
{
    AB_PROFILE;
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::PlayerQuestList ql;
    ql.uuid = player.data_.uuid;
    if (!client->Read(ql))
        return false;

    Game::Components::QuestComp& questComp = *player.questComp_;
    for (const auto& q : ql.questUuids)
    {
        AB::Entities::PlayerQuest pq;
        pq.uuid = q;
        if (!client->Read(pq))
            continue;
        if (pq.deleted)
            continue;

        AB::Entities::Quest quest;
        quest.uuid = pq.questUuid;
        if (!client->Read(quest))
            continue;

        questComp.Add(quest, std::move(pq));
    }
    return true;
}

static bool SaveQuestLog(Game::Player& player)
{
    auto* client = GetSubsystem<IO::DataClient>();
    Game::Components::QuestComp& questComp = *player.questComp_;
    questComp.VisitQuests([client](Game::Quest& current)
    {
        current.SaveProgress();
        client->UpdateOrCreate(current.playerQuest_);
        return Iteration::Continue;
    });
    return true;
}

static bool LoadPlayer(Game::Player& player)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(player.data_))
    {
        LOG_ERROR << "Error reading player data" << std::endl;
        return false;
    }
    player.account_.uuid = player.data_.accountUuid;
    if (!client->Read(player.account_))
    {
        LOG_ERROR << "Error reading player account" << std::endl;
        return false;
    }

    if (Utils::Uuid::IsEmpty(player.data_.professionUuid))
    {
        LOG_ERROR << "Error primary profession is nil" << std::endl;
        return false;
    }
    player.skills_->prof1_.uuid = player.data_.professionUuid;
    if (!client->Read(player.skills_->prof1_))
    {
        LOG_ERROR << "Error reading player profession1" << std::endl;
        return false;
    }
    if (!Utils::Uuid::IsEmpty(player.data_.profession2Uuid))
    {
        player.skills_->prof2_.uuid = player.data_.profession2Uuid;
        if (!client->Read(player.skills_->prof2_))
        {
            LOG_ERROR << "Error reading player profession2" << std::endl;
            return false;
        }
    }
    // After loading professions we can load the skills
    if (!player.skills_->Load(player.data_.skillTemplate, player.account_.type >= AB::Entities::AccountTypeGamemaster))
        LOG_WARNING << "Unable to decode skill template " << player.data_.skillTemplate << std::endl;

    player.inventoryComp_->SetInventorySize(player.data_.inventory_size);
    player.inventoryComp_->SetChestSize(player.account_.chest_size);
    if (!LoadPlayerInventory(player))
        return false;
    if (!LoadQuestLog(player))
        return false;
    return true;
}

bool LoadCharacter(AB::Entities::Character& ch)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(ch))
    {
        LOG_ERROR << "Error reading player data" << std::endl;
        return false;
    }
    return true;
}

bool LoadPlayerByName(Game::Player& player, const std::string& name)
{
    player.data_.name = name;
    return LoadPlayer(player);
}

bool LoadPlayerByUuid(Game::Player& player, const std::string& uuid)
{
    player.data_.uuid = uuid;
    return LoadPlayer(player);
}

bool SavePlayer(Game::Player& player)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    player.data_.lastLogin = player.loginTime_;
    player.data_.lastLogout = player.logoutTime_;
    player.data_.profession2 = player.skills_->prof2_.abbr;
    player.data_.profession2Uuid = player.skills_->prof2_.uuid;
    player.data_.skillTemplate = player.skills_->Encode();
    player.data_.onlineTime += static_cast<int64_t>((player.logoutTime_ - player.loginTime_) / 1000);
    if (!client->Update(player.data_))
        return false;
    if (!SavePlayerInventory(player))
        return false;
    if (!SaveQuestLog(player))
        return false;
    return true;
}

size_t GetInterestedParties(const std::string& accountUuid, std::vector<std::string>& accounts)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return 0;

    accounts.clear();
    std::unordered_set<std::string> ignored;

    // I friended those
    AB::Entities::FriendList fl;
    fl.uuid = accountUuid;
    if (client->Read(fl))
    {
        for (const auto& f : fl.friends)
        {
            if (f.relation == AB::Entities::FriendRelationFriend)
                accounts.push_back(f.friendUuid);
            else if (f.relation == AB::Entities::FriendRelationIgnore)
                ignored.emplace(f.friendUuid);
        }
    }
    // Those friended me
    AB::Entities::FriendedMe fme;
    fme.uuid = accountUuid;
    if (client->Read(fme))
    {
        for (const auto& f : fme.friends)
        {
            if (f.relation == AB::Entities::FriendRelationFriend)
                accounts.push_back(f.accountUuid);
            else if (f.relation == AB::Entities::FriendRelationIgnore)
                ignored.emplace(f.accountUuid);
        }
    }

    auto isIgnored = [&ignored](const std::string& uuid)
    {
        const auto it = ignored.find(uuid);
        return it != ignored.end();
    };

    if (!Utils::Uuid::IsEmpty(acc.guildUuid))
    {
        // If this guy is in a guild, also inform guild members
        AB::Entities::GuildMembers gms;
        gms.uuid = acc.guildUuid;
        if (client->Read(gms))
        {
            for (const auto& gm : gms.members)
            {
                if (!Utils::Uuid::IsEqual(accountUuid, gm.accountUuid) && !isIgnored(gm.accountUuid))
                    // Don't add self and ignored
                    accounts.push_back(gm.accountUuid);
            }
        }
    }
    std::sort(accounts.begin(), accounts.end());
    accounts.erase(std::unique(accounts.begin(), accounts.end()), accounts.end());
    return accounts.size();
}

bool GetPlayerInfoByName(const std::string& name, AB::Entities::Character& player)
{
    auto* client = GetSubsystem<IO::DataClient>();
    player.name = name;
    return client->Read(player);
}

bool GetPlayerInfoByAccount(const std::string& accountUuid, AB::Entities::Character& player)
{
    auto* client = GetSubsystem<IO::DataClient>();
    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return false;
    if (!Utils::Uuid::IsEmpty(acc.currentCharacterUuid))
        player.uuid = acc.currentCharacterUuid;
    else if (acc.characterUuids.size() != 0)
        player.uuid = acc.characterUuids[0];
    else
        return false;
    return client->Read(player);
}

bool IsIgnoringMe(const std::string& meUuid, const std::string& uuid)
{
    return GetRelationToMe(meUuid, uuid) == AB::Entities::FriendRelationIgnore;
}

bool HasFriendedMe(const std::string& meUuid, const std::string& uuid)
{
    return GetRelationToMe(meUuid, uuid) == AB::Entities::FriendRelationFriend;
}

AB::Entities::FriendRelation GetRelationToMe(const std::string& meUuid, const std::string& uuid)
{
    // Get realtion of `uuid` to `meUuid`
    auto* client = GetSubsystem<IO::DataClient>();

    // Those have me in their friendlist, be it friended or ignored
    AB::Entities::FriendedMe fme;
    fme.uuid = meUuid;
    if (client->Read(fme))
    {
        for (const auto& f : fme.friends)
        {
            if (Utils::Uuid::IsEqual(uuid, f.accountUuid))
                return f.relation;
        }
    }
    return AB::Entities::FriendRelationUnknown;
}

}
}
