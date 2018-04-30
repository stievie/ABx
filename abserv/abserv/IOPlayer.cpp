#include "stdafx.h"
#include "IOPlayer.h"
#include "PropStream.h"
#include "IOGame.h"
#include "Utils.h"
#include "ConfigManager.h"
#include "DataClient.h"
#include <AB/Entities/Account.h>
#include "Profiler.h"

#include "DebugNew.h"

namespace IO {

bool IOPlayer::PreloadPlayer(Game::Player* player, const std::string& name)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    player->data_.name = name;
    return client->Read(player->data_);
}

bool IOPlayer::LoadCharacter(AB::Entities::Character& player)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    return client->Read(player);
}

bool IOPlayer::LoadPlayerByName(Game::Player* player, const std::string& name)
{
    player->data_.name = name;
    if (!LoadCharacter(player->data_))
        return false;
    return true;
}

bool IOPlayer::SavePlayer(Game::Player* player)
{
    AB_PROFILE;
    IO::DataClient* client = Application::Instance->GetDataClient();
    player->data_.lastLogin = player->loginTime_;
    player->data_.lastLogout = player->logoutTime_;
    player->data_.onlineTime += static_cast<int64_t>((player->logoutTime_ - player->loginTime_) / 1000);
    return client->Update(player->data_);

#if 0
    Database* db = Database::Instance();

    // Conditions
    IO::PropWriteStream stream;
    for (auto& effect : player->effects_)
    {
        if (effect->IsPersistent())
        {
            effect->Serialize(stream);
            stream.Write<uint8_t>(Game::EffectAttrEnd);
        }
    }
    size_t effectSize;
    const char* effectBlob = stream.GetStream(effectSize);

    std::ostringstream query;
    query << "UPDATE `players` SET ";
    query << " `level` = " << player->data_.level << ",";
    query << " `experience` = " << player->data_.xp << ",";
    query << " `skillpoints` = " << player->data_.skillPoints << ",";
    query << " `lastlogin` = " << player->loginTime_ << ",";
    query << " `lastlogout` = " << player->logoutTime_ << ",";
    query << " `lastip` = " << player->client_->GetIP() << ",";
    // Online time in sec
    query << " `onlinetime` = `onlinetime` + " << static_cast<int>((player->logoutTime_ - player->loginTime_) / 1000) << ",";

    query << " `effects` = " << db->EscapeBlob(effectBlob, static_cast<uint32_t>(effectSize)) << ",";

    query << " `last_map` = " << db->EscapeString(player->data_.lastMap);

    query << " WHERE `id` = " << player->data_.id;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
#endif
}

IOPlayer::CreatePlayerResult IOPlayer::CreatePlayer(const std::string& accountUuid,
    std::string& name, const std::string& prof, AB::Entities::CharacterSex sex, bool isPvp)
{
    IO::DataClient* client = Application::Instance->GetDataClient();

    AB::Entities::Account acc;
    acc.uuid = accountUuid;
    if (!client->Read(acc))
        return ResultInvalidAccount;
    if (acc.characterUuids.size() + 1 > acc.charSlots)
        return ResultNoMoreCharSlots;

    AB::Entities::Character ch;
    ch.name = name;
    if (client->Exists(ch))
        return ResultNameExists;
    // To reload the character list
    client->Invalidate(acc);

    const uuids::uuid guid = uuids::uuid_system_generator{}();
    ch.uuid = guid.to_string();
    ch.name = name;
    ch.profession = prof;
    ch.sex = sex;
    ch.pvp = isPvp;
    ch.level = isPvp ? static_cast<uint8_t>(ConfigManager::Instance[ConfigManager::Key::PlayerLevelCap].GetInt()) : 1;
    ch.creation = Utils::AbTick();
    ch.accountUuid = accountUuid;
    if (!client->Create(ch))
        return ResultInternalError;

    return ResultOK;
}

bool IOPlayer::DeletePlayer(const std::string& accountUuid, const std::string& playerUuid)
{
    IO::DataClient* client = Application::Instance->GetDataClient();
    AB::Entities::Character ch;
    ch.uuid = playerUuid;
    if (!client->Read(ch))
        return false;
    if (ch.accountUuid.compare(accountUuid) != 0)
        return false;
    return client->Delete(ch);
}

}