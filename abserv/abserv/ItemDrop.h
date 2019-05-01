#pragma once

#include "GameObject.h"

namespace Game {

class Item;
class Actor;

class ItemDrop : public GameObject
{
private:
    std::unique_ptr<Item> item_;
    bool pickedUp_;
    /// Dropper
    std::weak_ptr<Actor> source_;
    void PickUp(Actor* actor);
public:
    static void RegisterLua(kaguya::State& state);

    explicit ItemDrop(std::unique_ptr<Item>& item);
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
    const Item* GetItem() const { return item_ ? item_.get() : nullptr; }

    void OnSelected(Actor* actor) override;
    void OnClicked(Actor* actor) override;
    void SetSource(std::shared_ptr<Actor> source);
    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;

    uint32_t actorId_;
};

}
