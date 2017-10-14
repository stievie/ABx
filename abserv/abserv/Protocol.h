#pragma once

#include <memory>
#include "Connection.h"

class Protocol
{
private:
    std::shared_ptr<Connection> connection_;
public:
    Protocol(std::shared_ptr<Connection> connection) :
        connection_(connection)
    {}
    Protocol(const Protocol&) = delete;
    ~Protocol();


};

