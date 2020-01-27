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

#ifdef USE_PGSQL

#include "Database.h"
#include <libpq-fe.h>

namespace DB {

class DatabasePgsql final : public Database
{
protected:
    PGconn* handle_;
    std::string dns_;
    bool Connect(int numTries = 1);
    bool InternalQuery(const std::string& query) final;
    std::shared_ptr<DBResult> InternalSelectQuery(const std::string& query) final;
    std::string Parse(const std::string& s);
public:
    DatabasePgsql();
    ~DatabasePgsql() override;

    bool BeginTransaction() final;
    bool Rollback() final;
    bool Commit() final;

    bool GetParam(DBParam param) final;
    uint64_t GetLastInsertId() final;
    std::string EscapeString(const std::string& s) final;
    std::string EscapeBlob(const char* s, size_t length) final;
    void FreeResult(DBResult* res) final;
    void CheckConnection() override;
};

class PgsqlResult final : public DBResult
{
    friend class DatabasePgsql;
protected:
    explicit PgsqlResult(PGresult* res);

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
