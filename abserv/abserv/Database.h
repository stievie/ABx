#pragma once

namespace DB {

class Database
{
private:
    static Database* instance_;
public:
    Database();
    virtual ~Database();
};

}
