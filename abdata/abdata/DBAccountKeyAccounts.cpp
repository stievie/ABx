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


#include "DBAccountKeyAccounts.h"

namespace DB {

bool DBAccountKeyAccounts::Create(AB::Entities::AccountKeyAccounts& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid) || Utils::Uuid::IsEmpty(ak.accountUuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();
    std::ostringstream query;
    query << "INSERT INTO account_account_keys (account_uuid, account_key_uuid) VALUES ( ";
    query << db->EscapeString(ak.accountUuid) << ",";
    query << db->EscapeString(ak.uuid);
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

bool DBAccountKeyAccounts::Load(AB::Entities::AccountKeyAccounts& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid) || Utils::Uuid::IsEmpty(ak.accountUuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT * FROM account_account_keys WHERE ";
    query << "account_uuid = " << db->EscapeString(ak.accountUuid);
    query << " AND account_key_uuid = " << db->EscapeString(ak.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    ak.uuid = result->GetString("account_key_uuid");
    ak.accountUuid = result->GetString("account_uuid");
    return true;
}

bool DBAccountKeyAccounts::Save(const AB::Entities::AccountKeyAccounts&)
{
    // Not possible
    return true;
}

bool DBAccountKeyAccounts::Delete(const AB::Entities::AccountKeyAccounts&)
{
    // No possible
    return true;
}

bool DBAccountKeyAccounts::Exists(const AB::Entities::AccountKeyAccounts& ak)
{
    if (Utils::Uuid::IsEmpty(ak.uuid) || Utils::Uuid::IsEmpty(ak.accountUuid))
    {
        LOG_ERROR << "UUID is empty" << std::endl;
        return false;
    }

    Database* db = GetSubsystem<Database>();

    std::ostringstream query;
    query << "SELECT COUNT(*) AS count FROM account_account_keys WHERE ";
    query << "account_uuid = " << db->EscapeString(ak.accountUuid);
    query << " AND account_key_uuid = " << db->EscapeString(ak.uuid);

    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    return result->GetUInt("count") != 0;
}

}
