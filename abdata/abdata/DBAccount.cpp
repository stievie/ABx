#include "stdafx.h"
#include "DBAccount.h"
#include "Database.h"
#include <uuid.h>
#include "Subsystems.h"

namespace DB {

bool DBAccount::Create(AB::Entities::Account& account)
{
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO `accounts` (`uuid`, `name`, `password`, `email`, `type`, " <<
        "`status`, `creation`, `char_slots`, `current_server_uuid`, `online_status`, `guild_uuid`, `chest_size`) VALUES ( ";

    query << db->EscapeString(account.uuid) << ", ";
    query << db->EscapeString(account.name) << ", ";
    query << db->EscapeString(account.password) << ", ";
    query << db->EscapeString(account.email) << ", ";
    query << static_cast<int>(account.type) << ", ";
    query << static_cast<int>(account.status) << ", ";
    query << account.creation << ", ";
    query << account.charSlots << ", ";
    query << db->EscapeString(account.currentServerUuid) << ", ";
    query << static_cast<int>(account.onlineStatus) << ", ";
    query << db->EscapeString(account.guildUuid) << ", ";
    query << static_cast<int>(account.chest_size);

    query << ")";
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    if (!transaction.Commit())
        return false;

    return true;
}

bool DBAccount::Load(AB::Entities::Account& account)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `accounts` WHERE ";
    if (!Utils::Uuid::IsEmpty(account.uuid))
        query << "`uuid` = " << db->EscapeString(account.uuid);
    else if (!account.name.empty())
        query << "`name` = " << db->EscapeString(account.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
    {
        LOG_ERROR << "No record found for " << query.str() << std::endl;
        return false;
    }

    account.uuid = result->GetString("uuid");
    account.name = result->GetString("name");
    account.password = result->GetString("password");
    account.email = result->GetString("email");
    account.authToken = result->GetString("auth_token");
    account.authTokenExpiry = result->GetLong("auth_token_expiry");
    account.type = static_cast<AB::Entities::AccountType>(result->GetInt("type"));
    account.status = static_cast<AB::Entities::AccountStatus>(result->GetInt("status"));
    account.creation = result->GetLong("creation");
    account.charSlots = result->GetUInt("char_slots");
    account.currentCharacterUuid = result->GetString("current_character_uuid");
    account.currentServerUuid = result->GetString("current_server_uuid");
    account.onlineStatus = static_cast<AB::Entities::OnlineStatus>(result->GetInt("online_status"));
    account.guildUuid = result->GetString("guild_uuid");
    account.chest_size = static_cast<uint16_t>(result->GetInt("chest_size"));

    // load characters
    account.characterUuids.clear();
    query.str("");
    query << "SELECT `uuid`, `name` FROM `players` WHERE `account_uuid` = " << db->EscapeString(account.uuid) <<
        " ORDER BY `name`";
    for (result = db->StoreQuery(query.str()); result; result = result->Next())
    {
        account.characterUuids.push_back(result->GetString("uuid"));
    }

    return true;
}

bool DBAccount::Save(const AB::Entities::Account& account)
{
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `accounts` SET ";

    query << " `password` = " << db->EscapeString(account.password) << ",";
    query << " `email` = " << db->EscapeString(account.email) << ",";
    query << " `auth_token` = " << db->EscapeString(account.authToken) << ",";
    query << " `auth_token_expiry` = " << account.authTokenExpiry << ",";
    query << " `type` = " << static_cast<int>(account.type) << ",";
    query << " `status` = " << static_cast<int>(account.status) << ",";
    query << " `char_slots` = " << account.charSlots << ",";
    query << " `current_character_uuid` = " << db->EscapeString(account.currentCharacterUuid) << ",";
    query << " `current_server_uuid` = " << db->EscapeString(account.currentServerUuid) << ",";
    query << " `online_status` = " << static_cast<int>(account.onlineStatus) << ",";
    query << " `guild_uuid` = " << db->EscapeString(account.guildUuid) << ",";
    query << " `chest_size` = " << static_cast<int>(account.chest_size);

    query << " WHERE `uuid` = " << db->EscapeString(account.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccount::Delete(const AB::Entities::Account& account)
{
    if (Utils::Uuid::IsEmpty(account.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "DELETE FROM `accounts` WHERE `uuid` = " << db->EscapeString(account.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccount::Exists(const AB::Entities::Account& account)
{
    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `accounts` WHERE ";
    if (!Utils::Uuid::IsEmpty(account.uuid))
        query << "`uuid` = " << db->EscapeString(account.uuid);
    else if (!account.name.empty())
        query << "`name` = " << db->EscapeString(account.name);
    else
    {
        LOG_ERROR << "UUID and name are empty" << std::endl;
        return false;
    }

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

bool DBAccount::LogoutAll()
{
    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "UPDATE `accounts` SET `online_status` = 0";
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

}
