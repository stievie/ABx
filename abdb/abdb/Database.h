/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <mutex>
#include <sstream>
#include <memory>

namespace DB {

class DBTransaction;
class DBQuery;
class DBResult;

enum DBParam
{
    DBPARAM_MULTIINSERT = 1
};

class Database
{
protected:
    Database() :
        connected_(false)
    {}

    friend class DBTransaction;
    virtual bool BeginTransaction() = 0;
    virtual bool Rollback() = 0;
    virtual bool Commit() = 0;

    virtual bool InternalQuery(const std::string& query) = 0;
    virtual std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) = 0;
    std::shared_ptr<DBResult> VerifyResult(std::shared_ptr<DBResult> result);
    bool connected_;
public:
    static std::string driver_;
    static std::string dbHost_;
    static std::string dbUser_;
    static std::string dbPass_;
    /// Name of Database or Database SQlite file
    static std::string dbName_;
    static uint16_t dbPort_;

    virtual ~Database() {}
    static Database* CreateInstance(const std::string& driver,
        const std::string& host, uint16_t port,
        const std::string& user, const std::string& pass,
        const std::string& name);

    virtual bool GetParam(DBParam) { return false; }
    bool IsConnected() const { return connected_; }

    bool ExecuteQuery(const std::string& query);
    std::shared_ptr<DBResult> StoreQuery(const std::string& query);
    virtual void FreeResult(DBResult* res);
    virtual uint64_t GetLastInsertId() = 0;
    virtual std::string EscapeString(const std::string& s) = 0;
    virtual std::string EscapeBlob(const char* s, size_t length) = 0;
    virtual bool CheckConnection() { return false; }
};

class DBResult : public std::enable_shared_from_this<DBResult>
{
protected:
    DBResult() = default;
    virtual ~DBResult();
public:
    virtual int32_t GetInt(const std::string&) {
        return 0;
    }
    virtual uint32_t GetUInt(const std::string&) {
        return 0;
    }
    virtual int64_t GetLong(const std::string&) {
        return 0;
    }
    virtual uint64_t GetULong(const std::string&) {
        return 0;
    }
    virtual time_t GetTime(const std::string&) {
        // time_t = int64_t = BIGINT(20)
        return 0;
    }
    virtual std::string GetString(const std::string&) {
        return "''";
    }
    virtual std::string GetStream(const std::string&) {
        return std::string("");
    }
    virtual bool IsNull(const std::string&) {
        return true;
    }

    virtual std::shared_ptr<DBResult> Next() { return std::shared_ptr<DBResult>(); }
    virtual bool Empty() const { return true; }
};

class DBTransaction
{
private:
    enum class State
    {
        Unknown,
        Started,
        Committed
    };
    Database* db_;
    State state_;
public:
    explicit DBTransaction(Database* db) :
        db_(db),
        state_(State::Unknown)
    {}
    ~DBTransaction()
    {
        if (state_ == State::Started)
        {
            db_->Rollback();
        }
    }
    bool Begin()
    {
        state_ = State::Started;
        return db_->BeginTransaction();
    }
    bool Commit()
    {
        if (state_ == State::Started)
        {
            state_ = State::Committed;
            return db_->Commit();
        }
        return false;
    }
};

}
