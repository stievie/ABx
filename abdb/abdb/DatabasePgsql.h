#pragma once

#ifdef USE_PGSQL

#include "Database.h"
#ifdef _WIN32
#include <libpq-fe.h>
#else
#include <postgresql/libpq-fe.h>
#endif

namespace DB {

class DatabasePgsql final : public Database
{
protected:
    PGconn* handle_;
    bool Connect(int numTries = 1);
    bool InternalQuery(const std::string& query) final;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) final;
    std::string Parse(const std::string& s);
public:
    DatabasePgsql();
    virtual ~DatabasePgsql();

    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, size_t length) final;
    void FreeResult(DBResult* res) final;
};

class PgsqlResult final : public DBResult
{
    friend class DatabasePgsql;
protected:
    PgsqlResult(PGresult* res);

    int32_t rows_, cursor_;
    PGresult* handle_;
public:
    virtual ~PgsqlResult();
    int32_t GetInt(const std::string& col) final;
    uint32_t GetUInt(const std::string& col) final;
    int64_t GetLong(const std::string& col) final;
    uint64_t GetULong(const std::string& col) final;
    time_t GetTime(const std::string& col) final;
    std::string GetString(const std::string& col) final;
    std::string GetStream(const std::string& col) final;
    bool IsNull(const std::string& col) final;

    bool Empty() const final { return cursor_ >= rows_; }
    std::shared_ptr<DBResult> Next() final;
};

}

#endif