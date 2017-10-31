#pragma once

#include <memory>
#include <ostream>
#include <sstream>
#include <mutex>

namespace DB {

class DBTransaction;

class Database
{
private:
    static Database* instance_;
protected:
    Database() :
        connected_(false)
    {}
    virtual ~Database() {}

    friend class DBTransaction;
    virtual bool BeginTransaction() = 0;
    virtual bool Rollback() = 0;
    virtual bool Commit() = 0;

    bool connected_;
public:
    static Database* Instance();

    bool IsConnected() const { return connected_; }
};

class DBResult : std::enable_shared_from_this<DBResult>
{

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
