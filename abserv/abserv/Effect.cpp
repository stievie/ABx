#include "stdafx.h"
#include "Effect.h"
#include "Creature.h"
#include "GameManager.h"
#include "Utils.h"

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
    GameManager::RegisterLuaAll(luaState_);
    luaState_["this"] = this;
}

bool Effect::LoadScript(const std::string& fileName)
{
    if (!luaState_.dofile(fileName.c_str()))
        return false;

    category_ = static_cast<EffectCategory>(luaState_["category"]);
    return true;
}

void Effect::Update(uint32_t timeElapsed)
{
    luaState_["onUpdate"](timeElapsed);
    if (endTime_ <= Utils::AbTick())
    {
        luaState_["onEnd"](target_);
        ended_ = true;
    }
}

void Effect::Start(std::shared_ptr<Creature> target, uint32_t ticks)
{
    target_ = target;
    startTime_ = Utils::AbTick();
    ticks_ = ticks;
    endTime_ = startTime_ + ticks;
    luaState_["onStart"](target_);
}

void Effect::Remove()
{
    luaState_["onRemove"]();
    cancelled_ = true;
}

bool Effect::Serialize(IO::PropWriteStream& stream)
{
    stream.Write<uint8_t>(EffectAttrId);
    stream.Write<uint32_t>(id_);

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
