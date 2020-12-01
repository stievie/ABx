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

#include "ItemDrop.h"
#include "Item.h"
#include "Actor.h"
#include "ItemFactory.h"
#include "Game.h"
#include "ItemsCache.h"
#include "Player.h"
#include <AB/Packets/Packet.h>
#include <AB/Packets/ServerPackets.h>
#include <sa/time.h>

namespace Game {

void ItemDrop::RegisterLua(kaguya::State& state)
{
    // clang-format off
    state["ItemDrop"].setClass(kaguya::UserdataMetatable<ItemDrop, GameObject>()
       .addFunction("GetSource", &ItemDrop::_LuaGetSource)
       .addFunction("GetTarget", &ItemDrop::_LuaGetTarget)
       .addFunction("GetItem", &ItemDrop::_LuaGetItem)
    );
    // clang-format on
}

ItemDrop::ItemDrop(uint32_t itemId) :
    GameObject(),
    itemId_(itemId),
    dropTick_(sa::time::tick())
{
    events_.Subscribe<void(Actor*)>(EVENT_ON_CLICKED, std::bind(&ItemDrop::OnClicked, this, std::placeholders::_1));
    // Drops can not hide other objects
    SetOccluder(false);
    selectable_ = true;
    auto* cache = GetSubsystem<ItemsCache>();
    auto* item = cache->Get(itemId);
    if (item)
    {
        name_ = item->data_.name;
        itemIndex_ = item->data_.index;
        concreteUuid_ = item->concreteItem_.uuid;
    }
}

ItemDrop::~ItemDrop()
{
    if (!pickedUp_ && itemId_ != 0)
    {
        // Not picked up delete it
        auto* factory = GetSubsystem<ItemFactory>();
        factory->DeleteConcrete(concreteUuid_);
    }
}

Actor* ItemDrop::_LuaGetSource()
{
    if (auto sharedSource = source_.lock())
        return sharedSource.get();
    return nullptr;
}

Actor* ItemDrop::_LuaGetTarget()
{
    if (targetId_ == 0)
        return nullptr;
    return GetGame()->GetObject<Actor>(targetId_);
}

Item* ItemDrop::_LuaGetItem()
{
    return const_cast<Item*>(GetItem());
}

void ItemDrop::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);
    using namespace sa::time::literals;
    if ((targetId_ != 0) && sa::time::time_elapsed(dropTick_) >= 300_s)
    {
        // If it wasn't picked up in 5 minutes it becomes available for all
        targetId_ = 0;
        AB::Packets::Server::DropTargetChanged packet = {
            id_, targetId_
        };
        message.AddByte(AB::GameProtocol::ServerPacketType::DropTargetChanged);
        AB::Packets::Add(packet, message);
    }
}

uint32_t ItemDrop::GetItemIndex() const
{
    return itemIndex_;
}

const Item* ItemDrop::GetItem() const
{
    auto* cache = GetSubsystem<ItemsCache>();
    return cache->Get(itemId_);
}

void ItemDrop::PickUp(Actor* actor)
{
    if (!actor)
        return;

    // if targetId_ == 0 all can pick it up
    if (targetId_ != 0 && targetId_ != actor->GetId())
    {
        if (Is<Player>(actor))
            To<Player>(actor)->SendError(AB::GameProtocol::PlayerErrorValue::DropForOtherPlayer);
        return;
    }

    if (actor->AddToInventory(itemId_))
    {
        pickedUp_ = true;
        if (actor->GetSelectedObjectId() == id_)
            actor->SelectedObjectById(0);
        Remove();
    }
}

uint32_t ItemDrop::GetSourceId()
{
    if (auto s = source_.lock())
        return s->id_;
    return 0;
}

void ItemDrop::OnClicked(Actor* actor)
{
    if (!actor)
        return;
    if (!IsInRange(Ranges::Adjecent, actor))
        return;

    PickUp(actor);
}

void ItemDrop::SetSource(ea::shared_ptr<Actor> source)
{
    source_ = source;
}

bool ItemDrop::Serialize(sa::PropWriteStream& stream)
{
    using namespace AB::GameProtocol;

    auto* cache = GetSubsystem<ItemsCache>();
    auto item = cache->Get(itemId_);
    if (!item)
        return false;

    static constexpr uint32_t validFields = ObjectSpawnDataFieldName | ObjectSpawnDataFieldModelIndex;
    stream.Write<uint32_t>(validFields);
    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint32_t>(item->data_.index);                   // Model index
    return true;
}

void ItemDrop::WriteSpawnData(Net::NetworkMessage& msg)
{
    GameObject::WriteSpawnData(msg);

    using namespace AB::GameProtocol;
    static constexpr uint32_t validFields = ObjectSpawnFieldPos | ObjectSpawnFieldRot | ObjectSpawnFieldScale |
        ObjectSpawnFieldUndestroyable | ObjectSpawnFieldSelectable | ObjectSpawnFieldState;
    msg.Add<uint32_t>(validFields);

    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<bool>(true);                                  // not destroyable
    msg.Add<bool>(selectable_);
    msg.Add<uint8_t>(static_cast<uint8_t>(stateComp_.GetState()));
    sa::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.Add(std::string(cData, dataSize));
}

}
