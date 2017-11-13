#include "stdafx.h"
#include "Database.h"
#include "ConfigManager.h"
#include "Logger.h"
#ifdef USE_MYSQL
#include "DatabaseMysql.h"
#endif // USE_MYSQL
#ifdef USE_PGSQL
#include "DatabasePgsql.h"
#endif // USE_PGSQL


#include "DebugNew.h"

namespace DB {

std::recursive_mutex DBQuery::lock_;
std::unique_ptr<Database> Database::instance_ = nullptr;

Database* Database::Instance()
{
    if (!instance_)
    {
        const std::string& driver = ConfigManager::Instance[ConfigManager::DBDriver].GetString();
#ifdef USE_MYSQL
        if (driver.compare("mysql") == 0)
            instance_ = std::make_unique<DatabaseMysql>();
#endif
#ifdef USE_PGSQL
        if (driver.compare("pgsql") == 0)
            instance_ = std::make_unique<DatabasePgsql>();
#endif
        if (!instance_)
        {
            LOG_ERROR << "Unknown/unsupported database driver " << driver << std::endl;
            return nullptr;
        }
    }
    return instance_.get();
}

bool Database::ExecuteQuery(const std::string& query)
{
    return InternalQuery(query);
}

bool Database::ExecuteQuery(DBQuery& query)
{
    return InternalQuery(query.str());
}

std::shared_ptr<DBResult> Database::StoreQuery(const std::string& query)
{
    return InternalSelectQuery(query);
}

std::shared_ptr<DBResult> Database::StoreQuery(DBQuery& query)
{
    return InternalSelectQuery(query.str());
}

std::shared_ptr<DBResult> Database::VerifyResult(std::shared_ptr<DBResult> result)
{
    if (!result->Next())
        return std::shared_ptr<DBResult>();
    return result;
}

void Database::FreeResult(DBResult*)
{
    throw std::runtime_error("No database driver loaded, yet a DBResult was freed.");
}

DBInsert::DBInsert(Database * db) :
    db_(db),
    rows_(0)
{
    // Checks if current database engine supports multi line INSERTs
    multiLine_ = db_->GetParam(DBPARAM_MULTIINSERT) != 0;
}

void DBInsert::SetQuery(const std::string& query)
{
    query_ = query;
    buff_.str("");
    rows_ = 0;
}

bool DBInsert::AddRow(const std::string& row)
{
    if (multiLine_) {
        ++rows_;
        size_t size = buff_.tellp();

        // adds new row to buffer
        if (size == 0)
            buff_ << "(" << row << ")";
        else if (size > 8192)
        {
            if (!Execute())
                return false;

            buff_ << "(" << row << ")";
        }
        else
            buff_ << ",(" + row + ")";

        return true;
    }

    // executes INSERT for current row
    return db_->ExecuteQuery(query_ + "(" + row + ")");
}

bool DBInsert::Execute()
{
    if (multiLine_ && buff_.tellp() > 0)
    {
        if (rows_ == 0)
            //no rows to execute
            return true;

        // executes buffer
        bool res = db_->ExecuteQuery(query_ + buff_.str());

        // Reset counters
        rows_ = 0;
        buff_.str("");
        return res;
    }

    // INSERTs were executed on-fly
    return true;
}

}
