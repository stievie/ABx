#pragma once

#include <memory>
#include "Vector3.h"
#include "Quaternion.h"
#include "Game.h"

namespace Game {

class GameObject : public std::enable_shared_from_this<GameObject>
{
protected:
    std::shared_ptr<Game> game_;
public:
    GameObject();
    ~GameObject();

    Math::Vector3 position_;
    Math::Quaternion rotation_;

};

}
