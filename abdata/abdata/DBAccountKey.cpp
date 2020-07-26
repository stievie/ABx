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

#include "DBAccountKey.h"

namespace DB {

bool DBAccountKey::Create(AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO account_keys (uuid, used, total, description, status, " <<
        "key_type, email) VALUES ( ";

    query << db->EscapeString(ak.uuid) << ", ";
    query << static_cast<int>(ak.used) << ", ";
    query << static_cast<int>(ak.total) << ", ";
    query << db->EscapeString(ak.description) << ", ";
    query << static_cast<int>(ak.status) << ", ";
    query << static_cast<int>(ak.type) << ", ";
    query << db->EscapeString(ak.email);

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

bool DBAccountKey::Load(AB::Entities::AccountKey& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM account_keys WHERE uuid = " << db->EscapeString(ak.uuid);
    if (ak.status != AB::Entities::AccountKeyStatus::KeyStatusUnknown)
        query << " AND status = " << ak.status;
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        query << " AND key_type = " << ak.type;

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
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;

    query << "UPDATE account_keys SET ";

    query << " used = " << ak.used << ",";
    query << " total = " << ak.total << ",";
    query << " description = " << db->EscapeString(ak.description) << ",";
    query << " status = " << static_cast<int>(ak.status) << ",";
    query << " key_type = " << static_cast<int>(ak.type) << ",";
    query << " email = " << db->EscapeString(ak.email);

    query << " WHERE uuid = " << db->EscapeString(ak.uuid);

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
    if (Utils::Uuid::IsEmpty(ak.uuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS count FROM account_keys WHERE uuid = " << db->EscapeString(ak.uuid);
    if (ak.status != AB::Entities::AccountKeyStatus::KeyStatusUnknown)
        query << " AND status = " << ak.status;
    if (ak.type != AB::Entities::AccountKeyType::KeyTypeUnknown)
        query << " AND key_type = " << ak.type;

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;
    return result->GetUInt("count") != 0;
}

}
