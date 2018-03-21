#include "stdafx.h"

#ifdef USE_ODBC

#ifndef _WIN32
#error ODBC Driver works only on Windows
#endif

#include "DatabaseOdbc.h"
#include "Logger.h"
#include "ConfigManager.h"
#include "DebugNew.h"

#define RETURN_SUCCESS(ret) (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)

namespace DB {

DatabaseOdbc::DatabaseOdbc() :
    Database(),
    transactions_(0)
{
    const std::string& sdsn = ConfigManager::Instance[ConfigManager::DBName];
    const std::string& suser = ConfigManager::Instance[ConfigManager::DBUser];
    const std::string& spass = ConfigManager::Instance[ConfigManager::DBPass];

    char dsn[SQL_MAX_DSN_LENGTH];
    char user[32];
    char pass[32];
    strcpy_s(dsn, sdsn.c_str());
    strcpy_s(user, suser.c_str());
    strcpy_s(pass, spass.c_str());

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env_);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "Failed to allocate ODBC SQLHENV enviroment handle." << std::endl;
        env_ = NULL;
        return;
    }

    ret = SQLSetEnvAttr(env_, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "SQLSetEnvAttr(SQL_ATTR_ODBC_VERSION): Failed to switch to ODBC 3 version." << std::endl;
        SQLFreeHandle(SQL_HANDLE_ENV, env_);
        env_ = NULL;
    }

    if (env_ == NULL)
    {
        LOG_ERROR << "ODBC SQLHENV enviroment not initialized." << std::endl;
        return;
    }

    ret = SQLAllocHandle(SQL_HANDLE_DBC, env_, &handle_);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "Failed to allocate ODBC SQLHDBC connection handle." << std::endl;
        handle_ = NULL;
        return;
    }

    ret = SQLSetConnectAttr(handle_, SQL_ATTR_CONNECTION_TIMEOUT, (SQLPOINTER*)5, 0);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "SQLSetConnectAttr(SQL_ATTR_CONNECTION_TIMEOUT): Failed to set connection timeout." << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, handle_);
        handle_ = NULL;
        return;
    }

    ret = SQLConnectA(handle_, (SQLCHAR*)&dsn, SQL_NTS,
        (SQLCHAR*)&user, SQL_NTS, (SQLCHAR*)&pass, SQL_NTS);
    if (!RETURN_SUCCESS(ret))
    {
        std::cout << "Failed to connect to ODBC via DSN: " << sdsn << " (user " << suser << ")" << std::endl;
        SQLFreeHandle(SQL_HANDLE_DBC, handle_);
        handle_ = NULL;
        return;
    }

    connected_ = true;
}

DatabaseOdbc::~DatabaseOdbc()
{
    if (connected_)
    {
        SQLDisconnect(handle_);
        SQLFreeHandle(SQL_HANDLE_DBC, handle_);
        handle_ = NULL;
        connected_ = false;
    }

    SQLFreeHandle(SQL_HANDLE_ENV, env_);
}

bool DatabaseOdbc::GetParam(DBParam param)
{
    switch (param)
    {
    case DBPARAM_MULTIINSERT:
        return false;
        break;

    default:
        return false;
    }
}

uint64_t DatabaseOdbc::GetLastInsertId()
{
    // https://blog.sqlauthority.com/2007/03/25/sql-server-identity-vs-scope_identity-vs-ident_current-retrieve-last-inserted-identity-of-record/
    std::ostringstream query;
    query << "SELECT SCOPE_IDENTITY() AS last_id";
    std::shared_ptr<DBResult> result = StoreQuery(query.str());
    if (result)
        return result->GetULong("last_id");
    return 0;
}

std::string DatabaseOdbc::EscapeString(const std::string& s)
{
    return EscapeBlob(s.c_str(), (uint32_t)s.length());
}

std::string DatabaseOdbc::EscapeBlob(const char* s, uint32_t length)
{
    std::string buf = "'";

    for (uint32_t i = 0; i < length; i++)
    {
        switch (s[i])
        {
        case '\'':
            buf += "\'\'";
            break;
        case '\0':
            buf += "\\0";
            break;
        case '\\':
            buf += "\\\\";
            break;
        case '\r':
            buf += "\\r";
            break;
        case '\n':
            buf += "\\n";
            break;
        default:
            buf += s[i];
        }
    }

    buf += "'";
    return buf;
}

void DatabaseOdbc::FreeResult(DBResult* res)
{
    delete (OdbcResult*)res;
}

bool DatabaseOdbc::BeginTransaction()
{
    if (!connected_)
        return false;
    bool ret = true;
    if (transactions_ == 0)
    {
        ret = RETURN_SUCCESS(SQLSetConnectAttrA(handle_, SQL_ATTR_AUTOCOMMIT,
            (SQLPOINTER)SQL_AUTOCOMMIT_OFF,
            SQL_IS_UINTEGER));
    }
    if (ret)
        transactions_++;
    return ret;
}

bool DatabaseOdbc::Rollback()
{
    if (!connected_)
        return false;
    if (transactions_ == 0)
        return false;
    bool ret = RETURN_SUCCESS(SQLEndTran(SQL_HANDLE_DBC, handle_, SQL_ROLLBACK));
    if (ret)
        transactions_--;
    if (transactions_ == 0)
    {
        SQLSetConnectAttrA(handle_, SQL_ATTR_AUTOCOMMIT,
            (SQLPOINTER)SQL_AUTOCOMMIT_ON,
            SQL_IS_UINTEGER);
    }
    return ret;
}

bool DatabaseOdbc::Commit()
{
    if (!connected_)
        return false;
    if (transactions_ == 0)
        return false;
    bool ret = RETURN_SUCCESS(SQLEndTran(SQL_HANDLE_DBC, handle_, SQL_COMMIT));
    if (ret)
        transactions_--;
    if (transactions_ == 0)
    {
        SQLSetConnectAttrA(handle_, SQL_ATTR_AUTOCOMMIT,
            (SQLPOINTER)SQL_AUTOCOMMIT_ON,
            SQL_IS_UINTEGER);
    }
    return ret;
}

bool DatabaseOdbc::InternalQuery(const std::string& query)
{
    if (!connected_)
        return false;

#ifdef DEBUG_SQL
    LOG_DEBUG << "ODBC QUERY: " << query << std::endl;
#endif

    std::string buf = Parse(query);

    SQLHSTMT stmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, handle_, &stmt);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "Failed to allocate ODBC SQLHSTMT statement." << std::endl;
        return false;
    }

    ret = SQLExecDirectA(stmt, (SQLCHAR*)buf.c_str(), (SQLINTEGER)buf.length());

    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "SQLExecDirect(): " << query << ": ODBC ERROR." << std::endl;
        return false;
    }

    return true;
}

std::shared_ptr<DBResult> DatabaseOdbc::InternalSelectQuery(const std::string& query)
{
    if (!connected_)
        return std::shared_ptr<DBResult>();

#ifdef DEBUG_SQL
    LOG_DEBUG << "ODBC QUERY: " << query << std::endl;
#endif

    std::string buf = Parse(query);

    SQLHSTMT stmt;

    SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, handle_, &stmt);
    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "Failed to allocate ODBC SQLHSTMT statement." << std::endl;
        return std::shared_ptr<DBResult>();
    }

    ret = SQLExecDirectA(stmt, (SQLCHAR*)buf.c_str(), (SQLINTEGER)buf.length());

    if (!RETURN_SUCCESS(ret))
    {
        LOG_ERROR << "SQLExecDirect(): " << query << ": ODBC ERROR." << std::endl;
        return std::shared_ptr<DBResult>();
    }

    std::shared_ptr<DBResult> results(new OdbcResult(stmt), std::bind(&Database::FreeResult, this, std::placeholders::_1));
    return VerifyResult(results);
}

std::string DatabaseOdbc::Parse(const std::string& s)
{
    std::string query = "";

    query.reserve(s.size());
    bool inString = false;
    uint8_t ch;
    for (uint32_t a = 0; a < s.length(); a++) {
        ch = s[a];

        if (ch == '\'') {
            if (inString && s[a + 1] != '\'')
                inString = false;
            else
                inString = true;
        }

        if (ch == '`' && !inString)
            ch = '"';

        query += ch;
    }

    return query;
}

OdbcResult::OdbcResult(SQLHSTMT stmt)
{
    handle_ = stmt;
    rowAvailable_ = false;

    int16_t numCols;
    SQLNumResultCols(handle_, &numCols);

    for (int32_t i = 1; i <= numCols; i++)
    {
        char* name = new char[129];
        SQLDescribeColA(handle_, (SQLUSMALLINT)i, (SQLCHAR*)name, (SQLUSMALLINT)129, NULL, NULL, NULL, NULL, NULL);
        listNames_[name] = i;
    }
}

OdbcResult::~OdbcResult()
{
    SQLFreeHandle(SQL_HANDLE_STMT, handle_);
}

int32_t OdbcResult::GetInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        int32_t value;
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_SLONG, &value, 0, NULL);

        if (RETURN_SUCCESS(ret))
            return value;
    }

    LOG_ERROR << "Error during GetInt(" << col << ")." << std::endl;
    return 0; // Failed}
}

uint32_t OdbcResult::GetUInt(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        uint32_t value;
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_ULONG, &value, 0, NULL);

        if (RETURN_SUCCESS(ret))
            return value;
    }

    LOG_ERROR << "Error during GetUInt(" << col << ")." << std::endl;
    return 0; // Failed}
}

int64_t OdbcResult::GetLong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        int64_t value;
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_SBIGINT, &value, 0, NULL);

        if (RETURN_SUCCESS(ret))
            return value;
    }

    LOG_ERROR << "Error during GetLong(" << col << ")." << std::endl;
    return 0; // Failed
}

uint64_t OdbcResult::GetULong(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        uint64_t value;
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_UBIGINT, &value, 0, NULL);

        if (RETURN_SUCCESS(ret))
            return value;
    }

    LOG_ERROR << "Error during GetULong(" << col << ")." << std::endl;
    return 0; // Failed
}

time_t OdbcResult::GetTime(const std::string& col)
{
    return static_cast<time_t>(GetLong(col));
}

std::string OdbcResult::GetString(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        char* value = new char[1024];

        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_CHAR, value, 1024, NULL);

        if (RETURN_SUCCESS(ret))
        {
            std::string buff = std::string(value);
            delete value;
            return buff;
        }
    }

    LOG_ERROR << "Error during GetString(" << col << ")." << std::endl;
    return std::string(""); // Failed
}

const char* OdbcResult::GetStream(const std::string& col, unsigned long& size)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        char* value = new char[1024];
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_BINARY,
            &value, 1024, (SQLLEN*)&size);

        if (RETURN_SUCCESS(ret))
            return value;
    }

    LOG_ERROR << "Error during GetStream(" << col << ")." << std::endl;
    size = 0;
    return nullptr; // Failed
}

bool OdbcResult::IsNull(const std::string& col)
{
    ListNames::iterator it = listNames_.find(col);
    if (it != listNames_.end())
    {
        uint16_t value;
        SQLRETURN ret = SQLGetData(handle_, (SQLUSMALLINT)it->second, SQL_C_SSHORT, &value, 0, NULL);

        if (RETURN_SUCCESS(ret))
            return value == SQL_NULL_DATA;
    }

    LOG_ERROR << "Error during IsNull(" << col << ")." << std::endl;
    return true; // Failed
}

std::shared_ptr<DBResult> OdbcResult::Next()
{
    SQLRETURN ret = SQLFetch(handle_);
    rowAvailable_ = RETURN_SUCCESS(ret);
    return rowAvailable_ ? shared_from_this() : std::shared_ptr<DBResult>();
}

}

#pragma comment(lib, "odbc32.lib")

#endif
