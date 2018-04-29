#pragma once

#include <AB/Entities/Mail.h>

namespace DB {

class DBMail
{
public:
    DBMail() = delete;
    ~DBMail() = delete;

    static bool Create(AB::Entities::Mail& mail);
    static bool Load(AB::Entities::Mail& mail);
    static bool Save(const AB::Entities::Mail& mail);
    static bool Delete(const AB::Entities::Mail& mail);
    static bool Exists(const AB::Entities::Mail& mail);
};

}
