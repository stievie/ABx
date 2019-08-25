#pragma once

#include "GameObject.h"

namespace Game {

class Item;
class Actor;

class ItemDrop final : public GameObject
{
private:
    uint32_t itemId_;
    uint32_t itemIndex_{ 0 };
    std::string concreteUuid_;
    bool pickedUp_{ false };
    /// Dropper
    std::weak_ptr<Actor> source_;
    void PickUp(Actor* actor);
protected:
    void OnClicked(Actor* actor);
    void OnSelected(Actor* actor);
public:
    static void RegisterLua(kaguya::State& state);

    explicit ItemDrop(uint32_t itemId);
    ItemDrop() = delete;
    // non-copyable
    ItemDrop(const ItemDrop&) = delete;
    ItemDrop& operator=(const ItemDrop&) = delete;

    ~ItemDrop() final;

    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypeItemDrop;
    }
    uint32_t GetItemIndex() const;
    const Item* GetItem() const;
    /// ID of dropper
    uint32_t GetSourceId();

    void SetSource(std::shared_ptr<Actor> source);
    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;


    uint32_t actorId_{ 0 };
};

}
