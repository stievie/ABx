#pragma once

#include <memory>
#include "Vector3.h"
#include "Quaternion.h"
#include <limits>

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
        return objectIds_++;
    }
public:
    GameObject();
    ~GameObject();

    virtual void Update(uint32_t timeElapsed) {
        UNREFERENCED_PARAMETER(timeElapsed);
    }

    Math::Vector3 position_;
    Math::Quaternion rotation_;
    /// Auto ID, not DB ID
    uint32_t id_;
};

}
