#pragma once

#ifdef USE_ODBC

#include "Database.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sqltypes.h>
#endif

#include <sql.h>
#include <sqlext.h>

namespace DB {

class DatabaseOdbc final : public Database
{
protected:
    SQLHDBC handle_;
    SQLHENV env_;
    bool InternalQuery(const std::string& query) final;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) final;
    std::string Parse(const std::string& s);
public:
    DatabaseOdbc();
    virtual ~DatabaseOdbc();

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, uint32_t length) final;
    void FreeResult(DBResult* res) final;
};

class OdbcResult : public DBResult
{
    friend class DatabaseOdbc;

protected:
    OdbcResult(SQLHSTMT stmt);

    typedef std::map<const std::string, uint32_t> ListNames;
    ListNames listNames_;
    bool rowAvailable_;

    SQLHSTMT handle_;
public:
    virtual ~OdbcResult();
};

}

#endif
