/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include "GameObject.h"
#include <sa/Noncopyable.h>

namespace Game {

class Item;
class Actor;

class ItemDrop final : public GameObject
{
    NON_COPYABLE(ItemDrop)
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

    ~ItemDrop() override;

    AB::GameProtocol::GameObjectType GetType() const override
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

template <>
inline bool Is<ItemDrop>(const GameObject& obj)
{
    return obj.GetType() == AB::GameProtocol::ObjectTypeItemDrop;
}

}
