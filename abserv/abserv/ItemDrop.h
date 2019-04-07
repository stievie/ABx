#pragma once

#include "GameObject.h"

namespace Game {

class Item;
class Actor;

class ItemDrop : public GameObject
{
private:
    std::unique_ptr<Item> item_;
    /// Dropper
    std::weak_ptr<Actor> source_;
public:
    static void RegisterLua(kaguya::State& state);

    explicit ItemDrop(std::unique_ptr<Item>& item);
    ItemDrop() = delete;
    // non-copyable
    ItemDrop(const ItemDrop&) = delete;
    ItemDrop& operator=(const ItemDrop&) = delete;

    ~ItemDrop() override;

    AB::GameProtocol::GameObjectType GetType() const final override
    {
        return AB::GameProtocol::ObjectTypeItemDrop;
    }
    uint32_t GetItemIndex() const;

    void OnClicked(Actor* actor) override;
    void SetSource(std::shared_ptr<Actor> source);
    bool Serialize(IO::PropWriteStream& stream) override;
    void WriteSpawnData(Net::NetworkMessage& msg) override;

    uint32_t actorId_;
};

}
