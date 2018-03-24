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

/// ODBC Database driver. Should only be used with MS SQL Server.
class DatabaseOdbc final : public Database
{
private:
    /// Number of active transactions.
    unsigned transactions_;
protected:
    SQLHDBC handle_;
    SQLHENV env_;
    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;
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
    int32_t GetInt(const std::string& col) final;
    uint32_t GetUInt(const std::string& col) final;
    int64_t GetLong(const std::string& col) final;
    uint64_t GetULong(const std::string& col) final;
    time_t GetTime(const std::string& col) final;
    std::string GetString(const std::string& col) final;
    std::string GetStream(const std::string& col) final;
    bool IsNull(const std::string& col) final;

    bool Empty() const final { return !rowAvailable_; }
    std::shared_ptr<DBResult> Next() final;
};

}

#endif
