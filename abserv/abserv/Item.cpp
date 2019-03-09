#include "stdafx.h"
#include "Item.h"
#include "DataProvider.h"
#include "ScriptManager.h"
#include "IOItem.h"
#include "Mechanic.h"

namespace Game {

void Item::RegisterLua(kaguya::State& state)
{
    state["Item"].setClass(kaguya::UserdataMetatable<Item>()
        .addFunction("GetWeaponRange", &Item::GetWeaponRange)
    );
}

void Item::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Item::LoadScript(const std::string& fileName)
{
    script_ = GetSubsystem<IO::DataProvider>()->GetAsset<Script>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;
    if (ScriptManager::IsFunction(luaState_, "onUpdate"))
        functions_ |= FunctionUpdate;
    if (ScriptManager::IsFunction(luaState_, "getDamage"))
        functions_ |= FunctionGetDamage;
    return true;
}

void Item::Update(uint32_t timeElapsed)
{
    if (HaveFunction(FunctionUpdate))
        luaState_["onUpdate"](timeElapsed);
    for (const auto& upg : upgrades_)
        if (upg.second)
            upg.second->Update(timeElapsed);
}

void Item::SetUpgrade(ItemUpgrade type, uint32_t index)
{
    AB::Entities::Item item;
    if (!IO::IOItem::LoadItemByIndex(item, index))
    {
        LOG_ERROR << "Failed to load item with index " << index << std::endl;
        return;
    }
    std::unique_ptr<Item> i = std::make_unique<Item>(item);
    if (i->LoadScript(item.script))
    {
        upgrades_[type] = std::move(i);
    }
}

void Item::RemoveUpgrade(ItemUpgrade type)
{
    if (upgrades_[type])
        upgrades_[type].reset();
}

float Item::GetWeaponRange() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
    case AB::Entities::ItemTypeSword:
    case AB::Entities::ItemTypeHammer:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeFlatbow:
        return RANGE_FLATBOW;
    case AB::Entities::ItemTypeHornbow:
        return RANGE_HORNBOW;
    case AB::Entities::ItemTypeShortbow:
        return RANGE_SHORTBOW;
    case AB::Entities::ItemTypeLongbow:
        return RANGE_LONGBOW;
    case AB::Entities::ItemTypeRecurvebow:
        return RANGE_RECURVEBOW;
    case AB::Entities::ItemTypeStaff:
    case AB::Entities::ItemTypeWand:
        return RANGE_PROJECTILE;
    case AB::Entities::ItemTypeDaggers:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeScyte:
        return RANGE_TOUCH;
    case AB::Entities::ItemTypeSpear:
        return RANGE_SPEAR;
    default:
        return 0.0f;
    }
}

uint32_t Item::GetWeaponAttackSpeed() const
{
    switch (data_.type)
    {
    case AB::Entities::ItemTypeAxe:
        return ATTACK_SPEED_AXE;
    case AB::Entities::ItemTypeSword:
        return ATTACK_SPEED_SWORD;
    case AB::Entities::ItemTypeHammer:
        return ATTACK_SPEED_HAMMER;
    case AB::Entities::ItemTypeFlatbow:
        return ATTACK_SPEED_FLATBOW;
    case AB::Entities::ItemTypeHornbow:
        return ATTACK_SPEED_HORNBOW;
    case AB::Entities::ItemTypeShortbow:
        return ATTACK_SPEED_SHORTBOW;
    case AB::Entities::ItemTypeLongbow:
        return ATTACK_SPEED_LONGBOW;
    case AB::Entities::ItemTypeRecurvebow:
        return ATTACK_SPEED_RECURVEBOW;
    case AB::Entities::ItemTypeStaff:
        return ATTACK_SPEED_STAFF;
    case AB::Entities::ItemTypeWand:
        return ATTACK_SPEED_WAND;
    case AB::Entities::ItemTypeDaggers:
        return ATTACK_SPEED_DAGGERS;
    case AB::Entities::ItemTypeScyte:
        return ATTACK_SPEED_SCYTE;
    case AB::Entities::ItemTypeSpear:
        return ATTACK_SPEED_SPEAR;
    default:
        return 0;
    }
}

}
