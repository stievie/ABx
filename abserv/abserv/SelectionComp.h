#pragma once

#include <stdint.h>
#include <memory>

namespace Net {
class NetworkMessage;
}

namespace Game {

class GameObject;
class Actor;

namespace Components {

class SelectionComp
{
private:
    Actor& owner_;
    uint32_t prevObjectId_{ 0 };
    uint32_t currObjectId_{ 0 };
public:
    SelectionComp() = delete;
    explicit SelectionComp(Actor& owner) :
        owner_(owner)
    { }
    // non-copyable
    SelectionComp(const SelectionComp&) = delete;
    SelectionComp& operator=(const SelectionComp&) = delete;
    ~SelectionComp();

    bool SelectObject(uint32_t targetId);
    bool ClickObject(uint32_t targetId);

    void Write(Net::NetworkMessage& message);
    uint32_t GetSelectedObjectId() const { return currObjectId_; }
    GameObject* GetSelectedObject() const;
};

}
}
