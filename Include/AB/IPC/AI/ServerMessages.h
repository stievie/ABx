#pragma once

#include <AB/IPC/Message.h>
#include <string>

namespace AI {

struct GameAdd : public IPC::Message<GameAdd>
{
    uint32_t id;
    std::string name;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(name);
    }
};

struct GameRemove : public IPC::Message<GameRemove>
{
    uint32_t id;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
    }
};

struct NpcUpdate : public IPC::Message<NpcUpdate>
{
    uint32_t id;
    std::string name;
    template<typename _Ar>
    void Serialize(_Ar& ar)
    {
        ar.value(id);
        ar.value(name);
    }
};

}
