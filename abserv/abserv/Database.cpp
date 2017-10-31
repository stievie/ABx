#include "stdafx.h"
#include "Database.h"
#include "ConfigManager.h"

#include "DebugNew.h"

namespace DB {

std::recursive_mutex DBQuery::lock_;
Database* Database::instance_ = nullptr;

Database* Database::Instance()
{
    if (!instance_)
    {

    }
    return instance_;
}

}
