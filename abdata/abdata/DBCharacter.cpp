#include "stdafx.h"
#include "DBCharacter.h"
#include "Database.h"

namespace DB {

bool DBCharacter::Create(AB::Entities::Character& character)
{
    Database* db = Database::Instance();
    std::ostringstream query;
    query << "INSERT INTO `players` (`pvp`) VALUES ( ";

    query << character.pvp;

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

bool DB::DBCharacter::Load(AB::Entities::Character& character)
{
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
        return false;

    DB::Database* db = DB::Database::Instance();

    std::ostringstream query;
    query << "SELECT * FROM `players` WHERE `uuid` = " << character.uuid;
    std::shared_ptr<DB::DBResult> result = db->StoreQuery(query.str());
    if (!result)
        return false;

    character.pvp = result->GetInt("pvp") != 0;

    return true;
}

bool DB::DBCharacter::Save(const AB::Entities::Character& character)
{
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
        return false;

    Database* db = Database::Instance();
    std::ostringstream query;

    query << "UPDATE `players` SET ";

    query << " `pvp` = " << character.pvp;

    query << " WHERE `uuid` = " << character.uuid;

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
    if (character.uuid.empty() || uuids::uuid(character.uuid).nil())
        return false;

    Database* db = Database::Instance();
    std::ostringstream query;
    query << "DELETE FROM `players` WHERE `uuid` = " << character.uuid;
    DBTransaction transaction(db);
    if (!transaction.Begin())
        return false;

    if (!db->ExecuteQuery(query.str()))
        return false;

    // End transaction
    return transaction.Commit();
}

}
