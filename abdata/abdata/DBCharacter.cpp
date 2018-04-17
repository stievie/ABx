#include "stdafx.h"
#include "DBCharacter.h"
#include "Database.h"

namespace DB {

uint32_t DBCharacter::Create(AB::Entities::Character& character)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `players` (`pvp`) VALUES ( ";

    query << character.pvp;

    query << ")";

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return 0;

    if (!db->ExecuteQuery(query.str()))
        return 0;

    // End transaction
    if (!transaction.Commit())
        return 0;

    character.id = static_cast<uint32_t>(db->GetLastInsertId());
    return character.id;
}

bool DB::DBCharacter::Load(AB::Entities::Character& character)
{
    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `players` WHERE `id` = " << character.id;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    character.pvp = result->GetInt("pvp") != 0;

    return true;
}

bool DB::DBCharacter::Save(const AB::Entities::Character& character)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    if (character.id == 0)
        return false;

    query << "UPDATE `players` SET ";

    query << " `pvp` = " << character.pvp;

    query << " WHERE `id` = " << character.id;

    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

bool DB::DBCharacter::Delete(const AB::Entities::Character& character)
{
    if (character.id == 0)
        return false;

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `players` WHERE `id` = " << character.id;
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

}
