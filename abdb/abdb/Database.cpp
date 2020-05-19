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

#include "stdafx.h"
#include "Database.h"
#ifdef USE_MYSQL
#include "DatabaseMysql.h"
#endif // USE_MYSQL
#ifdef USE_PGSQL
#include "DatabasePgsql.h"
#endif // USE_PGSQL
#ifdef USE_ODBC
#include "DatabaseOdbc.h"
#endif // USE_ODBC
#ifdef USE_SQLITE
#include "DatabaseSqlite.h"
#endif
#include <abscommon/Logger.h>

namespace DB {

std::string Database::driver_ = "";
std::string Database::dbHost_ = "";
std::string Database::dbName_ = "";
std::string Database::dbUser_ = "";
std::string Database::dbPass_ = "";
uint16_t Database::dbPort_ = 0;

Database* Database::CreateInstance(const std::string& driver,
    const std::string& host, uint16_t port,
    const std::string& user, const std::string& pass,
    const std::string& name)
{
    Database::driver_ = driver;
    Database::dbHost_ = host;
    Database::dbName_ = name;
    Database::dbUser_ = user;
    Database::dbPass_ = pass;
    Database::dbPort_ = port;
#ifdef USE_MYSQL
    if (driver.compare("mysql") == 0)
        return new DatabaseMysql();
#endif
#ifdef USE_PGSQL
    if (driver.compare("pgsql") == 0)
        return new DatabasePgsql();
#endif
#ifdef USE_ODBC
    if (driver.compare("odbc") == 0)
        return new DatabaseOdbc();
#endif
#ifdef USE_SQLITE
    if (driver.compare("sqlite") == 0)
        return new DatabaseSqlite(Database::dbName_);
#endif
    LOG_ERROR << "Unknown/unsupported database driver " << driver << std::endl;
    return nullptr;
}

bool Database::ExecuteQuery(const std::string& query)
{
    return InternalQuery(query);
}

std::shared_ptr<DBResult> Database::StoreQuery(const std::string& query)
{
    return InternalSelectQuery(query);
}

std::shared_ptr<DBResult> Database::VerifyResult(std::shared_ptr<DBResult> result)
{
    if (!result->Next())
        return std::shared_ptr<DBResult>();
    return result;
}

void Database::FreeResult(DBResult*)
{
    throw std::runtime_error("No database driver loaded, yet a DBResult was freed.");
}

DBResult::~DBResult() = default;

}
