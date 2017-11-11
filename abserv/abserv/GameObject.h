#pragma once

#include <memory>
#include "Vector3.h"
#include "Quaternion.h"
#include <limits>
#pragma warning(push)
#pragma warning(disable: 4702 4127 4244)
#include <kaguya/kaguya.hpp>
#pragma warning(pop)

namespace Game {

class Game;

class GameObject : public std::enable_shared_from_this<GameObject>
{
private:
    static uint32_t objectIds_;
protected:
    std::shared_ptr<Game> game_;
    uint32_t GetNewId()
    {
        if (objectIds_ >= std::numeric_limits<uint32_t>::max())
            objectIds_ = 0;
        return ++objectIds_;
    }
public:
    static void RegisterLua(kaguya::State& state);

    GameObject();
    virtual ~GameObject() = default;

    virtual void Update(uint32_t timeElapsed) {
        AB_UNUSED(timeElapsed);
    }

    uint32_t GetId() const { return id_; }

    Math::Vector3 position_;
    Math::Quaternion rotation_;
    /// Auto ID, not DB ID
    uint32_t id_;
};

}
