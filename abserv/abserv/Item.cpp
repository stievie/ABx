#include "stdafx.h"
#include "Item.h"
#include "DataProvider.h"
#include "ScriptManager.h"
#include "IOItem.h"

namespace Game {

void Item::RegisterLua(kaguya::State& state)
{
    state["Item"].setClass(kaguya::UserdataMetatable<Item>()
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

}
