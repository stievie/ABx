#include "stdafx.h"
#include "DBAccountKey.h"
#include "Database.h"
#include "Subsystems.h"

namespace DB {

bool DBAccountKey::Create(AB::Entities::AccountKey&)
{
    return true;
}

bool DBAccountKey::Load(AB::Entities::AccountKey& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM `account_keys` WHERE `uuid` = " << db->EscapeString(ak.uuid);
    if (ak.status != AB::Entities::AccountKeyStatus::Unknown)
        query << " AND `status` = " << ak.status;
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        query << " AND `key_type` = " << ak.type;

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ak.uuid = result->GetString("uuid");
    ak.used = static_cast<uint16_t>(result->GetUInt("used"));
    ak.total = static_cast<uint16_t>(result->GetUInt("total"));
    ak.description = result->GetString("description");
    ak.status = static_cast<AB::Entities::AccountKeyStatus>(result->GetUInt("status"));
    ak.type = static_cast<AB::Entities::AccountKeyType>(result->GetUInt("key_type"));
    ak.email = result->GetString("email");
    return true;
}

bool DBAccountKey::Save(const AB::Entities::AccountKey& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE `account_keys` SET ";

    query << " `used` = " << ak.used << ",";
    query << " `total` = " << ak.total << ",";
    query << " `description` = " << db->EscapeString(ak.description) << ",";
    query << " `status` = " << static_cast<int>(ak.status) << ",";
    query << " `key_type` = " << static_cast<int>(ak.type) << ",";
    query << " `email` = " << db->EscapeString(ak.email);

    query << " WHERE `uuid` = " << db->EscapeString(ak.uuid);

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DBAccountKey::Delete(const AB::Entities::AccountKey&)
{
    // Can not delete
    return true;
}

bool DBAccountKey::Exists(const AB::Entities::AccountKey& ak)
{
    if (ak.uuid.empty() || uuids::uuid(ak.uuid).nil())
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS `count` FROM `account_keys` WHERE `uuid` = " << db->EscapeString(ak.uuid);
    if (ak.status != AB::Entities::AccountKeyStatus::Unknown)
        query << " AND `status` = " << ak.status;
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        query << " AND `key_type` = " << ak.type;

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
