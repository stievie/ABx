#pragma once

#ifdef USE_MYSQL

#include "Database.h"
#ifdef _WIN32 // hack for broken mysql.h not including the correct winsock header for SOCKET definition, fixed in 5.7
#include <winsock2.h>
#endif
#include <mysql.h>

namespace DB {

class DatabaseMysql final : public Database
{
protected:
    MYSQL handle_;
    bool InternalQuery(const std::string& query) final;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) final;
public:
    DatabaseMysql();
    virtual ~DatabaseMysql();

    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, uint32_t length) final;
    void FreeResult(DBResult* res) final;
};

class MysqlResult final : public DBResult
{
    friend class DatabaseMysql;
protected:
    MysqlResult(MYSQL_RES* res);

    typedef std::map<const std::string, uint32_t> ListNames;
    ListNames listNames_;
    MYSQL_RES* handle_;
    MYSQL_ROW row_;
public:
    virtual ~MysqlResult();
    int32_t GetInt(const std::string& col) final;
    uint32_t GetUInt(const std::string& col) final;
    int64_t GetLong(const std::string& col) final;
    uint64_t GetULong(const std::string& col) final;
    time_t GetTime(const std::string& col) final;
    std::string GetString(const std::string& col) final;
    std::string GetStream(const std::string& col, unsigned long& size) final;
    bool IsNull(const std::string& col) final;

    bool Empty() const final { return row_ == NULL; }
    std::shared_ptr<DBResult> Next() final;
};

}

#endif
