#include "stdafx.h"

#ifdef USE_MYSQL

#include "DatabaseMysql.h"
#include "Logger.h"
#include <errmsg.h>
#include "ConfigManager.h"

#include "DebugNew.h"

namespace DB {

DatabaseMysql::DatabaseMysql() :
    Database()
{
    if (!mysql_init(&handle_))
    {
        LOG_ERROR << "Failed to initialize MySQL handle" << std::endl;
        return;
    }

    my_bool reconnect = true;
    mysql_options(&handle_, MYSQL_OPT_RECONNECT, &reconnect);

    const std::string& host = ConfigManager::Instance[ConfigManager::DBHost];
    const std::string& user = ConfigManager::Instance[ConfigManager::DBUser];
    const std::string& pass = ConfigManager::Instance[ConfigManager::DBPass];
    const std::string& db = ConfigManager::Instance[ConfigManager::DBName];
    const uint16_t port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::DBPort].GetInt());

    if (!mysql_real_connect(&handle_, host.c_str(), user.c_str(),
        pass.c_str(), db.c_str(), port, NULL, 0))
    {
        LOG_ERROR << "Failed to connect to MySQL database: " << mysql_error(&handle_) << std::endl;
        return;
    }


#if (MYSQL_VERSION_ID < 50019)
    // MySQL servers < 5.0.19 has a bug where MYSQL_OPT_RECONNECT is (incorrectly) reset by mysql_real_connect calls
    // See http://dev.mysql.com/doc/refman/5.0/en/mysql-options.html for more information.
    mysql_options(&handle_, MYSQL_OPT_RECONNECT, &reconnect);
#pragma message("Outdated MySQL server detected. Consider upgrading to a newer version.")
#endif

    connected_ = true;
}

DatabaseMysql::~DatabaseMysql()
{
    mysql_close(&handle_);
}

bool DatabaseMysql::BeginTransaction()
{
    return ExecuteQuery("BEGIN");
}

bool DatabaseMysql::Rollback()
{
    if (!connected_)
        return false;
#ifdef DEBUG_SQL
    LOG_DEBUG << "Rollback" << std::endl;
#endif

    if (mysql_rollback(&handle_) != 0)
    {
        LOG_ERROR << mysql_error(&handle_) << std::endl;
        return false;
    }
    return true;
}

bool DatabaseMysql::Commit()
{
    if (!connected_)
        return false;
#ifdef DEBUG_SQL
    LOG_DEBUG << "Commit" << std::endl;
#endif

    if (mysql_commit(&handle_) != 0)
    {
        LOG_ERROR << mysql_error(&handle_) << std::endl;
        return false;
    }
    return true;
}

bool DatabaseMysql::GetParam(DBParam param)
{
    switch (param)
    {
    case DBPARAM_MULTIINSERT:
        return true;
    default:
        return false;
    }
}

uint64_t DatabaseMysql::GetLastInsertId()
{
    std::ostringstream query;
    query << "SELECT LAST_INSERT_ID() AS last_id";
    std::shared_ptr<DBResult> result = StoreQuery(query.str());
    if (result)
        return result->GetULong("last_id");
    return 0;

    // Always returns 0?
//    return static_cast<uint64_t>(mysql_insert_id(&handle_));
}

std::string DatabaseMysql::EscapeString(const std::string& s)
{
    return EscapeBlob(s.c_str(), static_cast<uint32_t>(s.length()));
}

std::string DatabaseMysql::EscapeBlob(const char* s, uint32_t length)
{
    // remember about quoting even an empty string!
    if (!s)
        return std::string("''");

    // the worst case is 2n + 1
    char* output = new char[length * 2 + 1];

    // quotes escaped string and frees temporary buffer
    mysql_real_escape_string(&handle_, output, s, length);
    std::stringstream r;
    r << "'" << output << "'";
    delete[] output;
    return r.str();
}

void DatabaseMysql::FreeResult(DBResult* res)
{
    delete (MysqlResult*)res;
}

bool DatabaseMysql::InternalQuery(const std::string& query)
{
    if (!connected_)
        return false;

#ifdef DEBUG_SQL
    LOG_DEBUG << "MYSQL QUERY: " << query << std::endl;
#endif

    bool state = true;

    // executes the query
    if (mysql_real_query(&handle_, query.c_str(), static_cast<unsigned long>(query.length())) != 0)
    {
        LOG_ERROR << "mysql_real_query(): " << query.substr(0, 256) << ": MYSQL ERROR: " << mysql_error(&handle_) << std::endl;
        int error = mysql_errno(&handle_);

        if (error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
        {
            connected_ = false;
        }

        state = false;
    }

    // we should call that every time as someone would call executeQuery('SELECT...')
    // as it is described in MySQL manual: "it doesn't hurt" :P
    MYSQL_RES* m_res = mysql_store_result(&handle_);

    if (m_res)
        mysql_free_result(m_res);

    return state;
}

std::shared_ptr<DBResult> DatabaseMysql::InternalSelectQuery(const std::string& query)
{
    if (!connected_)
        return std::shared_ptr<DBResult>();

#ifdef DEBUG_SQL
    LOG_DEBUG << "MYSQL QUERY: " << query << std::endl;
#endif

    // executes the query
    if (mysql_real_query(&handle_, query.c_str(), static_cast<unsigned long>(query.length())) != 0)
    {
        LOG_ERROR << "mysql_real_query(): " << query << ": MYSQL ERROR: " << mysql_error(&handle_) << std::endl;
        int error = mysql_errno(&handle_);

        if (error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
        {
            connected_ = false;
        }
    }

    // we should call that every time as someone would call executeQuery('SELECT...')
    // as it is described in MySQL manual: "it doesn't hurt" :P
    MYSQL_RES* m_res = mysql_store_result(&handle_);

    // error occurred
    if (!m_res)
    {
        LOG_ERROR << "mysql_store_result(): " << query.substr(0, 256) << ": MYSQL ERROR: " << mysql_error(&handle_) << std::endl;
        int error = mysql_errno(&handle_);

        if (error == CR_SERVER_LOST || error == CR_SERVER_GONE_ERROR)
        {
            connected_ = false;
        }

        return std::shared_ptr<DBResult>();
    }

    // retrieving results of query
    std::shared_ptr<DBResult> res(new MysqlResult(m_res), std::bind(&Database::FreeResult, this, std::placeholders::_1));
    return VerifyResult(res);
}

MysqlResult::MysqlResult(MYSQL_RES* res)
{
    handle_ = res;
    listNames_.clear();

    MYSQL_FIELD* field;
    int i = 0;
    while ((field = mysql_fetch_field(handle_)) != NULL)
    {
        listNames_[field->name] = i;
        ++i;
    }
}

MysqlResult::~MysqlResult()
{
    mysql_free_result(handle_);
}

int32_t MysqlResult::GetInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        if (row_[it->second])
            return atoi(row_[it->second]);
        return 0;
    }

    LOG_ERROR << "Error in GetInt(" << col << ")" << std::endl;
    return 0; // Failed
}

uint32_t MysqlResult::GetUInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        if (row_[it->second])
        {
            std::istringstream os(row_[it->second]);
            uint32_t res;
            os >> res;
            return res;
        }
        return 0;
    }

    LOG_ERROR << "Error in GetUInt(" << col << ")" << std::endl;
    return 0; // Failed
}

int64_t MysqlResult::GetLong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        if (row_[it->second])
            return atoll(row_[it->second]);
        return 0;
    }

    LOG_ERROR << "Error in GetLong(" << col << ")" << std::endl;
    return 0; // Failed
}

uint64_t MysqlResult::GetULong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        if (row_[it->second])
            return strtoul(row_[it->second], nullptr, 0);
        return 0;
    }

    LOG_ERROR << "Error in GetULong(" << col << ")" << std::endl;
    return 0; // Failed
}

time_t MysqlResult::GetTime(const std::string& col)
{
    return static_cast<time_t>(GetLong(col));
}

std::string MysqlResult::GetString(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        if (row_[it->second])
            return std::string(row_[it->second]);
        return std::string("");
    }

    LOG_ERROR << "Error in GetString(" << col << ")" << std::endl;
    return std::string(""); // Failed
}

const char* MysqlResult::GetStream(const std::string& col, unsigned long& size)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end()) {
        if (row_[it->second])
        {
            size = mysql_fetch_lengths(handle_)[it->second];
            return row_[it->second];
        }
        size = 0;
        return nullptr;
    }

    LOG_ERROR << "Error in GetStream(" << col << ")" << std::endl;
    size = 0;
    return nullptr;
}

bool MysqlResult::IsNull(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        return (row_[it->second] == nullptr);
    }
    return true;
}

std::shared_ptr<DBResult> MysqlResult::Next()
{
    row_ = mysql_fetch_row(handle_);
    return row_ != NULL ? shared_from_this() : std::shared_ptr<DBResult>();
}

}

// Adds dependencies to
// * libmysql.dll
#pragma comment(lib, "libmysql.lib")

#endif
