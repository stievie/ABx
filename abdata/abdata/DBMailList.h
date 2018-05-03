#pragma once

#include <AB/Entities/MailList.h>

namespace DB {

class DBMailList
{
public:
    DBMailList() = delete;
    ~DBMailList() = delete;

    static bool Create(AB::Entities::MailList& ml);
    static bool Load(AB::Entities::MailList& ml);
    static bool Save(const AB::Entities::MailList& ml);
    static bool Delete(const AB::Entities::MailList& ml);
    static bool Exists(const AB::Entities::MailList& ml);
};

}
