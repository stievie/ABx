#pragma once

#include "Database.h"
#include <sqlite3.h>

namespace DB {

class DatabaseSqlite final : public Database
{
protected:
    sqlite3* handle_;
    std::recursive_mutex lock_;
    std::string Parse(const std::string& s);
    virtual bool InternalQuery(const std::string& query);
    virtual std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query);
public:
    DatabaseSqlite(const std::string& file);
    virtual ~DatabaseSqlite();

    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, uint32_t length) final;
    void FreeResult(DBResult* res) final;
};

class SqliteResult final : public DBResult
{
    friend class DatabaseSqlite;
protected:
    SqliteResult(sqlite3_stmt* res);

    typedef std::map<const std::string, uint32_t> ListNames;
    ListNames listNames_;
    sqlite3_stmt* handle_;
    bool rowAvailable_;
public:
    virtual ~SqliteResult();
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

