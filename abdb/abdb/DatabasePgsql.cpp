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



#ifdef USE_PGSQL

#include "DatabasePgsql.h"
#include <abscommon/Logger.h>
#include <sa/Assert.h>

namespace DB {

DatabasePgsql::DatabasePgsql() :
    Database(),
    handle_(nullptr)
{
    const std::string& host = Database::dbHost_;
    const std::string& user = Database::dbUser_;
    const std::string& pass = Database::dbPass_;
    const std::string& db = Database::dbName_;
    const uint16_t port = Database::dbPort_;

    std::stringstream dns;
    dns << "host='" << host << "' dbname='" << db << "' user='" << user <<
        "' password='" << pass << "' port='" << port << "'";
    dns_ = dns.str();

    if (!Connect(10))
    {
        LOG_ERROR << "Unable to connect to PostgresSQL database: " <<
            PQerrorMessage(handle_) << std::endl;
        return;
    }
}

DatabasePgsql::~DatabasePgsql()
{
    PQfinish(handle_);
}

bool DatabasePgsql::Connect(int numTries /* = 1 */)
{
    int tr = 0;
    while (!connected_ && tr < numTries)
    {
        int remaingTries = tr - numTries;
        PGPing ping = PQping(dns_.c_str());
        switch (ping)
        {
        case PQPING_REJECT:
            LOG_ERROR << "Server rejected connection, trying for " << remaingTries << " more times" << std::endl;
            break;
        case PQPING_NO_RESPONSE:
            LOG_ERROR << "Server didn't respond, trying for " << remaingTries << " more times" << std::endl;
            break;
        case PQPING_NO_ATTEMPT:
            LOG_ERROR << "Bad parameters, check connection parameters" << std::endl;
            return false;
        default:
            break;
        }

        if (ping == PQPING_OK)
        {
            // When ping OK then try to connect
            handle_ = PQconnectdb(dns_.c_str());
            connected_ = PQstatus(handle_) == CONNECTION_OK;
        }
        ++tr;
        if (!connected_ && remaingTries > 0)
            std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return connected_;
}

bool DatabasePgsql::BeginTransaction()
{
    return ExecuteQuery("BEGIN");
}

bool DatabasePgsql::Rollback()
{
#ifdef DEBUG_SQL
    LOG_DEBUG << "Rollback" << std::endl;
#endif
    return ExecuteQuery("ROLLBACK");
}

bool DatabasePgsql::Commit()
{
#ifdef DEBUG_SQL
    LOG_DEBUG << "Commit" << std::endl;
#endif
    return ExecuteQuery("COMMIT");
}

bool DatabasePgsql::GetParam(DBParam param)
{
    switch (param)
    {
    case DBPARAM_MULTIINSERT:
        return true;
    default:
        return false;
    }
}

uint64_t DatabasePgsql::GetLastInsertId()
{
    if (!connected_)
        return 0;

    PGresult* res = PQexec(handle_, "SELECT LASTVAL() as last;");
    ExecStatusType stat = PQresultStatus(res);

    if (stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "PQexec(): failed to fetch last row: " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        return 0;
    }

    uint64_t id = strtoul(PQgetvalue(res, 0, PQfnumber(res, "last")), nullptr, 0);
    PQclear(res);
    return id;
}

std::string DatabasePgsql::EscapeString(const std::string& s)
{
    if (!s.size())
        return std::string("''");

    int32_t error;
    char* output = new char[s.length() * 2 + 1];

    PQescapeStringConn(handle_, output, s.c_str(), s.length(), reinterpret_cast<int*>(&error));
    std::stringstream r;
    r << "'" << output << "'";
    delete[] output;
    return r.str();
}

std::string DatabasePgsql::EscapeBlob(const char* s, size_t length)
{
    if (!s || length == 0)
        return std::string("''");

    std::stringstream r;
    r << "'" << base64::encode((const unsigned char*)s, length) << "'";
    return r.str();
}

void DatabasePgsql::FreeResult(DBResult* res)
{
    delete (PgsqlResult*)res;
}

void DatabasePgsql::CheckConnection()
{
    if (dns_.empty())
        return;

    PGresult* res = PQexec(handle_, "SELECT 1");
    ExecStatusType stat = PQresultStatus(res);
    if (stat == PGRES_COMMAND_OK || stat == PGRES_TUPLES_OK)
        // OK
        return;

    connected_ = false;
    PGPing ping = PQping(dns_.c_str());
    if (ping == PQPING_OK)
        Connect();
}

bool DatabasePgsql::InternalQuery(const std::string& query)
{
    if (!connected_)
        return false;

#ifdef DEBUG_SQL
    LOG_DEBUG << "PGSQL QUERY: " << query << std::endl;
#endif

    PGresult* res = PQexec(handle_, Parse(query).c_str());
    ExecStatusType stat = PQresultStatus(res);

    if (stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "PQexec(): " << query << ": " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        return false;
    }

    PQclear(res);
    return true;
}

std::shared_ptr<DBResult> DatabasePgsql::InternalSelectQuery(const std::string& query)
{
    if (!connected_)
        return std::shared_ptr<DBResult>();

#ifdef DEBUG_SQL
    LOG_DEBUG << "PGSQL QUERY: " << query << std::endl;
#endif

    // executes query
    PGresult* res = PQexec(handle_, Parse(query).c_str());
    ExecStatusType stat = PQresultStatus(res);

    if (stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "PQexec(): " << query << ": " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        return std::shared_ptr<DBResult>();
    }

    // everything went fine
    std::shared_ptr<DBResult> results(new PgsqlResult(res), std::bind(&Database::FreeResult, this, std::placeholders::_1));
    return VerifyResult(results);
}

std::string DatabasePgsql::Parse(const std::string& s)
{
    std::string query;
    query.reserve(s.length());

    bool inString = false;
    uint8_t ch;
    for (size_t a = 0; a < s.length(); ++a)
    {
        ch = s[a];

        if (ch == '\'')
        {
            if (inString && s[a + 1] != '\'')
                inString = false;
            else
                inString = true;
        }

        if (ch == '`' && !inString)
            ch = '"';

        query += ch;
    }

    return query;
}

PgsqlResult::PgsqlResult(PGresult* res) :
    cursor_(-1),
    handle_(res)
{
    rows_ = PQntuples(handle_) - 1;
}

PgsqlResult::~PgsqlResult()
{
    PQclear(handle_);
}

int32_t PgsqlResult::GetInt(const std::string& col)
{
    int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    return strtol(PQgetvalue(handle_, cursor_, column), nullptr, 10);
}

uint32_t PgsqlResult::GetUInt(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    return strtoul(PQgetvalue(handle_, cursor_, column), nullptr, 10);
}

int64_t PgsqlResult::GetLong(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    return strtoll(PQgetvalue(handle_, cursor_, column), nullptr, 10);
}

uint64_t PgsqlResult::GetULong(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    return strtoull(PQgetvalue(handle_, cursor_, column), nullptr, 10);
}

time_t PgsqlResult::GetTime(const std::string& col)
{
    return static_cast<time_t>(GetLong(col));
}

std::string PgsqlResult::GetString(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    size_t size = PQgetlength(handle_, cursor_, column);
    return std::string(PQgetvalue(handle_, cursor_, column), size);
}

std::string PgsqlResult::GetStream(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    size_t size = PQgetlength(handle_, cursor_, column);
    char* buf = PQgetvalue(handle_, cursor_, column);
    return base64::decode(buf, size);
}

bool PgsqlResult::IsNull(const std::string& col)
{
    const int column = PQfnumber(handle_, col.c_str());
    assert(column >= 0);
    return PQgetisnull(handle_, cursor_, column) != 0;
}

std::shared_ptr<DBResult> PgsqlResult::Next()
{
    if (cursor_ >= rows_)
        return std::shared_ptr<DBResult>();

    ++cursor_;
    return shared_from_this();
}

}

#endif
