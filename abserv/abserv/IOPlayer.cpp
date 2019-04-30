#include "stdafx.h"
#include "IOPlayer.h"
#include "PropStream.h"
#include "IOGame.h"
#include "Utils.h"
#include "ConfigManager.h"
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include "Profiler.h"
#include <AB/Entities/Profession.h>
#include <AB/Entities/PlayerItemList.h>
#include "Logger.h"
#include <uuid.h>
#include "Subsystems.h"
#include "SkillManager.h"

#include "DebugNew.h"

namespace IO {

bool IOPlayer::LoadPlayer(Game::Player* player)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    if (!client->Read(player->data_))
    {
        LOG_ERROR << "Error reading player data" << std::endl;
        return false;
    }
    player->account_.uuid = player->data_.accountUuid;
    if (!client->Read(player->account_))
    {
        LOG_ERROR << "Error reading player account" << std::endl;
        return false;
    }

    if (uuids::uuid(player->data_.professionUuid).nil())
    {
        LOG_ERROR << "Error primary profession is nil" << std::endl;
        return false;
    }
    player->skills_->prof1_.uuid = player->data_.professionUuid;
    if (!client->Read(player->skills_->prof1_))
    {
        LOG_ERROR << "Error reading player profession1" << std::endl;
        return false;
    }
    if (!uuids::uuid(player->data_.profession2Uuid).nil())
    {
        player->skills_->prof2_.uuid = player->data_.profession2Uuid;
        if (!client->Read(player->skills_->prof2_))
        {
            LOG_ERROR << "Error reading player profession2" << std::endl;
            return false;
        }
    }
    // After loading professions we can load the skills
    if (!player->skills_->Load(player->data_.skillTemplate, player->account_.type >= AB::Entities::AccountTypeGamemaster))
    {
        LOG_WARNING << "Unable to decode skill template " << player->data_.skillTemplate << std::endl;
        // TODO: Remove
        if (player->account_.type >= AB::Entities::AccountTypeGamemaster)
        {
            LOG_INFO << "Adding GM skills" << std::endl;
            auto skillsMan = GetSubsystem<Game::SkillManager>();
            player->skills_->SetSkill(0, skillsMan->Get(9998));
            player->skills_->SetSkill(1, skillsMan->Get(9997));
            player->skills_->SetSkill(2, skillsMan->Get(9996));
            player->skills_->SetSkill(3, skillsMan->Get(312));
            player->skills_->SetSkill(4, skillsMan->Get(240));
            player->skills_->SetSkill(5, skillsMan->Get(281));
            player->skills_->SetSkill(7, skillsMan->Get(1043));
        }
    }

    player->inventoryComp_->SetSize(player->data_.inventory_size);
    if (!LoadPlayerInventory(player))
        return false;
    return true;
}

bool IOPlayer::LoadCharacter(AB::Entities::Character& ch)
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

bool IOPlayer::LoadPlayerByName(Game::Player* player, const std::string& name)
{
    player->data_.name = name;
    return LoadPlayer(player);
}

bool IOPlayer::LoadPlayerByUuid(Game::Player* player, const std::string& uuid)
{
    player->data_.uuid = uuid;
    return LoadPlayer(player);
}

bool IOPlayer::SavePlayer(Game::Player* player)
{
    AB_PROFILE;
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    player->data_.lastLogin = player->loginTime_;
    player->data_.lastLogout = player->logoutTime_;
    player->data_.profession2 = player->skills_->prof2_.abbr;
    player->data_.profession2Uuid = player->skills_->prof2_.uuid;
    player->data_.skillTemplate = player->skills_->Encode();
    player->data_.onlineTime += static_cast<int64_t>((player->logoutTime_ - player->loginTime_) / 1000);
    if (!client->Update(player->data_))
        return false;
    return SavePlayerInventory(player);
}

bool IOPlayer::LoadPlayerInventory(Game::Player* player)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();

    AB::Entities::EquippedItems equipmenet;
    equipmenet.uuid = player->data_.uuid;
    if (client->Read(equipmenet))
    {
        for (const auto& e : equipmenet.itemUuids)
        {
            player->SetEquipment(e);
        }
    }

    AB::Entities::InventoryItems inventory;
    inventory.uuid = player->data_.uuid;
    if (client->Read(inventory))
    {
        for (const auto& e : inventory.itemUuids)
        {
            player->SetInventory(e);
        }
    }
    return true;
}

bool IOPlayer::SavePlayerInventory(Game::Player* player)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    player->inventoryComp_->VisitEquipement([client](Game::Item* item)
    {
        client->Update(item->concreteItem_);
        client->Invalidate(item->concreteItem_);
    });

    player->inventoryComp_->VisitInventory([client](Game::Item* item)
    {
        client->Update(item->concreteItem_);
        client->Invalidate(item->concreteItem_);
    });

    AB::Entities::InventoryItems inventory;
    inventory.uuid = player->data_.uuid;
    client->Invalidate(inventory);
    return true;
}

}
