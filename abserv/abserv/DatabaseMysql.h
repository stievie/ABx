#pragma once

#include "Database.h"
#include <winsock2.h>              // SOCKET
#include <mysql.h>
#include <map>

namespace DB {

class DatabaseMysql : public Database
{
protected:
    MYSQL handle_;
    bool InternalQuery(const std::string& query) override;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) override;
public:
    DatabaseMysql();
    virtual ~DatabaseMysql();

    bool BeginTransaction() override;
    bool Rollback() override;
    bool Commit() override;

    bool GetParam(DBParam param) override;
    uint64_t GetLastInsertId() override;
    std::string EscapeString(const std::string& s) override;
    std::string EscapeBlob(const char* s, uint32_t length) override;
    void FreeResult(DBResult* res) override;
};

class MySqlResult : public DBResult
{
    friend class DatabaseMysql;
protected:
    MySqlResult(MYSQL_RES* res);
    virtual ~MySqlResult();

    typedef std::map<const std::string, uint32_t> ListNames;
    ListNames listNames_;
    MYSQL_RES* handle_;
    MYSQL_ROW row_;
public:
    int32_t GetInt(const std::string& col) override;
    uint32_t GetUInt(const std::string& col) override;
    int64_t GetLong(const std::string& col) override;
    std::string GetString(const std::string& col) override;
    const char* GetStream(const std::string& col, unsigned long &size) override;

    bool Empty() override { return row_ == NULL; }
    std::shared_ptr<DBResult> Advance() override;
};

}
