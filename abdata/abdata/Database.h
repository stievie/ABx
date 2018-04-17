#pragma once

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
private:
    static std::unique_ptr<Database> instance_;
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
    static Database* Instance();
    static void Reset();

    virtual bool GetParam(DBParam) {
        return false;
    }
    bool IsConnected() const { return connected_; }

    bool ExecuteQuery(const std::string& query);
    bool ExecuteQuery(DBQuery& query);
    std::shared_ptr<DBResult> StoreQuery(const std::string& query);
    std::shared_ptr<DBResult> StoreQuery(DBQuery& query);
    virtual void FreeResult(DBResult* res);
    virtual uint64_t GetLastInsertId() = 0;
    virtual std::string EscapeString(const std::string& s) = 0;
    virtual std::string EscapeBlob(const char* s, uint32_t length) = 0;
};

class DBResult : public std::enable_shared_from_this<DBResult>
{
protected:
    DBResult() = default;
    virtual ~DBResult() {}
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

class DBQuery : public std::ostringstream
{
protected:
    static std::recursive_mutex lock_;
public:
    DBQuery()
    {
        lock_.lock();
    }
    ~DBQuery()
    {
        lock_.unlock();
    }
    void Reset() { str(""); }
};

class DBInsert
{
private:
    Database* db_;
    uint32_t rows_;
    bool multiLine_;
    std::string query_;
    std::ostringstream buff_;
public:
    DBInsert(Database* db);
    ~DBInsert() {};

    void SetQuery(const std::string& query);
    bool AddRow(const std::string& row);
    bool Execute();
};

class DBTransaction
{
private:
    enum TransactionState
    {
        StateNoStart,
        StateStart,
        StateCommit
    };
    TransactionState state_;
    Database* db_;
public:
    DBTransaction(Database* db) :
        db_(db),
        state_(StateNoStart)
    {}
    ~DBTransaction()
    {
        if (state_ == StateStart)
        {
            db_->Rollback();
        }
    }
    bool Begin()
    {
        state_ = StateStart;
        return db_->BeginTransaction();
    }
    bool Commit()
    {
        if (state_ == StateStart)
        {
            state_ = StateCommit;
            return db_->Commit();
        }
        return false;
    }
};

}
