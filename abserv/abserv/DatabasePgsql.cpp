#include "stdafx.h"
#include "DatabasePgsql.h"
#include "ConfigManager.h"
#include "Logger.h"

#include "DebugNew.h"

namespace DB {

DatabasePgsql::DatabasePgsql() :
    Database()
{
    const std::string& host = ConfigManager::Instance[ConfigManager::DBHost];
    const std::string& user = ConfigManager::Instance[ConfigManager::DBUser];
    const std::string& pass = ConfigManager::Instance[ConfigManager::DBPass];
    const std::string& db = ConfigManager::Instance[ConfigManager::DBName];
    const uint16_t port = static_cast<uint16_t>(ConfigManager::Instance[ConfigManager::DBPort].GetInt());
    std::stringstream dns;
    dns << "host='" << host << "' dbname='" << db << "' user='" << user <<
        "' password='" << pass << "' port='" << port << "'";
    handle_ = PQconnectdb(dns.str().c_str());
    connected_ = PQstatus(handle_) == CONNECTION_OK;

    if (!connected_)
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

bool DatabasePgsql::BeginTransaction()
{
    return ExecuteQuery("BEGIN");
}

bool DatabasePgsql::Rollback()
{
    return ExecuteQuery("ROLLBACK");
}

bool DatabasePgsql::Commit()
{
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

    // everything went fine
    uint64_t id = strtoul(PQgetvalue(res, 0, PQfnumber(res, "last")), nullptr, 0);
    PQclear(res);
    return id;
}

std::string DatabasePgsql::EscapeString(const std::string& s)
{
    // remember to quote even empty string!
    if (!s.size())
        return std::string("''");

    // the worst case is 2n + 1
    int32_t error;
    char* output = new char[s.length() * 2 + 1];

    // quotes escaped string and frees temporary buffer
    PQescapeStringConn(handle_, output, s.c_str(), s.length(), reinterpret_cast<int*>(&error));
    std::stringstream r;
    r << "'" << output << "'";
    delete[] output;
    return r.str();
}

std::string DatabasePgsql::EscapeBlob(const char* s, uint32_t length)
{
    // remember to quote even empty stream!
    if (!s)
        return std::string("''");

    // quotes escaped string and frees temporary buffer
    size_t len;
    char* output = (char*)PQescapeByteaConn(handle_, (unsigned char*)s, length, &len);
    std::stringstream r;
    r << "E'" << output << "'";
    PQfreemem(output);
    return r.str();
}

void DatabasePgsql::FreeResult(DBResult* res)
{
    delete (PgsqlResult*)res;
}

bool DatabasePgsql::InternalQuery(const std::string& query)
{
    if (!connected_)
        return false;

   // executes query
    PGresult* res = PQexec(handle_, Parse(query).c_str());
    ExecStatusType stat = PQresultStatus(res);

    if (stat != PGRES_COMMAND_OK && stat != PGRES_TUPLES_OK)
    {
        LOG_ERROR << "PQexec(): " << query << ": " << PQresultErrorMessage(res) << std::endl;
        PQclear(res);
        return false;
    }

    // everything went fine
    PQclear(res);
    return true;
}

std::shared_ptr<DBResult> DatabasePgsql::InternalSelectQuery(const std::string& query)
{
    if (!connected_)
        return std::shared_ptr<DBResult>();

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
    std::string query = "";

    bool inString = false;
    uint8_t ch;
    for (uint32_t a = 0; a < s.length(); a++)
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
    handle_(res),
    cursor_(-1)
{
    rows_ = PQntuples(handle_) - 1;
}

PgsqlResult::~PgsqlResult()
{
    PQclear(handle_);
}

int32_t PgsqlResult::GetInt(const std::string& col)
{
    return atoi(PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str())));
}

uint32_t PgsqlResult::GetUInt(const std::string& col)
{
    return (uint32_t)atoi(PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str())));
}

int64_t PgsqlResult::GetLong(const std::string& col)
{
    return atoll(PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str())));
}

uint64_t PgsqlResult::GetULong(const std::string& col)
{
    return strtoul(PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str())), nullptr, 0);
}

time_t PgsqlResult::GetTime(const std::string& col)
{
    return static_cast<time_t>(GetLong(col));
}

std::string PgsqlResult::GetString(const std::string& col)
{
    return std::string(PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str())));
}

const char* PgsqlResult::GetStream(const std::string& col, unsigned long& size)
{
    std::string buf = PQgetvalue(handle_, cursor_, PQfnumber(handle_, col.c_str()));
    unsigned char* temp = PQunescapeBytea((const unsigned char*)buf.c_str(), (size_t*)&size);
    char* value = new char[buf.size()];
    strcpy_s(value, buf.size(), (char*)temp);
    PQfreemem(temp);
    return value;
}

bool PgsqlResult::IsNull(const std::string & col)
{
    return PQgetisnull(handle_, cursor_, PQfnumber(handle_, col.c_str())) != 0;
}

std::shared_ptr<DBResult> PgsqlResult::Next()
{
    if (cursor_ >= rows_)
        return std::shared_ptr<DBResult>();

    cursor_++;
    return shared_from_this();
}

}

// Adds dependencies to
// * libpq.dll
// * ssleay32.dll
// * libeay32.dll
// * libintl-8.dll
// * libiconv-2.dll
#pragma comment(lib, "libpq.lib")
