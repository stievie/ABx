#pragma once

#include <memory>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Skill : public std::enable_shared_from_this<Skill>
{
public:
    static void RegisterLua(kaguya::State& state);

    Skill();
    ~Skill();

    uint32_t costEnergy_;
    uint32_t costAdrenaline_;
    uint32_t activation_;
    uint32_t recharge_;
    uint32_t overcast_;
};

}
