#include "stdafx.h"
#include "Effect.h"
#include "Actor.h"
#include "ScriptManager.h"
#include "Utils.h"
#include "DataProvider.h"

namespace Game {

void Effect::RegisterLua(kaguya::State& state)
{
    state["Effect"].setClass(kaguya::UserdataMetatable<Effect>()
        .addFunction("GetStartTime", &Effect::GetStartTime)
        .addFunction("GetEndTime", &Effect::GetEndTime)
        .addFunction("GetTicks", &Effect::GetTicks)
    );
}

void Effect::InitializeLua()
{
    ScriptManager::RegisterLuaAll(luaState_);
    luaState_["self"] = this;
}

bool Effect::LoadScript(const std::string& fileName)
{
    script_ = IO::DataProvider::Instance.GetAsset<LuaScript>(fileName);
    if (!script_)
        return false;
    if (!script_->Execute(luaState_))
        return false;

    persistent_ = luaState_["isPersistent"];
    return true;
}

void Effect::Update(uint32_t timeElapsed)
{
    luaState_["onUpdate"](source_.lock(), target_.lock(), timeElapsed);
    if (endTime_ <= Utils::AbTick())
    {
        luaState_["onEnd"](source_.lock(), target_.lock());
        ended_ = true;
    }
}

void Effect::Start(std::shared_ptr<Actor> source, std::shared_ptr<Actor> target, uint32_t baseDuration)
{
    target_ = target;
    source_ = source;
    startTime_ = Utils::AbTick();
    ticks_ = luaState_["getDuration"](source_.lock(), target_.lock(), baseDuration);
    endTime_ = startTime_ + ticks_;
    luaState_["onStart"](source_.lock(), target_.lock());
}

void Effect::Remove()
{
    luaState_["onRemove"](source_.lock(), target_.lock());
    cancelled_ = true;
}

bool Effect::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(EffectAttrId);
    stream.Write<uint32_t>(data_.index);

    stream.Write<uint8_t>(EffectAttrTicks);
    stream.Write<uint32_t>(ticks_);

    return true;
}

bool Effect::Unserialize(IO::PropReadStream& stream)
{
    uint8_t attr;
    while (stream.Read<uint8_t>(attr) && attr != EffectAttrEnd)
    {
        if (!UnserializeProp(static_cast<EffectAttr>(attr), stream))
            return false;
    }
    return true;
}

bool Effect::UnserializeProp(EffectAttr attr, IO::PropReadStream& stream)
{
    switch (attr)
    {
    case EffectAttrId:
        break;
    case EffectAttrTicks:
        return stream.Read<uint32_t>(ticks_);
    }
    return false;
}

}
