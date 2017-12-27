#include "stdafx.h"

#ifdef USE_ODBC

#include "DatabaseOdbc.h"
#include "Logger.h"

#define RETURN_SUCCESS(ret) (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)

namespace DB {

DatabaseOdbc::DatabaseOdbc() :
    Database()
{
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
    return 0;  // TODO
}

std::string DatabaseOdbc::EscapeString(const std::string & s)
{
    return std::string();
}

std::string DatabaseOdbc::EscapeBlob(const char * s, uint32_t length)
{
    return std::string();
}

void DatabaseOdbc::FreeResult(DBResult* res)
{
    delete (OdbcResult*)res;
}

bool DatabaseOdbc::InternalQuery(const std::string& query)
{
    return false;
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

    ret = SQLExecDirectA(stmt, (SQLCHAR*)buf.c_str(), buf.length());

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
        SQLDescribeColA(handle_, i, (SQLCHAR*)name, 129, NULL, NULL, NULL, NULL, NULL);
        listNames_[name] = i;
    }
}

OdbcResult::~OdbcResult()
{
    SQLFreeHandle(SQL_HANDLE_STMT, handle_);
}

}

#endif
