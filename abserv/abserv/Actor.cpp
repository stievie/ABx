#include "stdafx.h"
#include "Actor.h"
#include "EffectManager.h"
#include "Game.h"
#include <AB/ProtocolCodes.h>
#include "Logger.h"
#include "OctreeQuery.h"
#include "GameManager.h"
#include "MathUtils.h"
#include "ScriptManager.h"
#include "ConfigManager.h"
#include "TemplateEncoder.h"

#include "DebugNew.h"

namespace Game {

void Actor::RegisterLua(kaguya::State& state)
{
    state["Actor"].setClass(kaguya::UserdataMetatable<Actor, GameObject>()
        .addFunction("IsTrigger", &Actor::IsTrigger)
        .addFunction("SetTrigger", &Actor::SetTrigger)

        .addFunction("GetLevel", &Actor::GetLevel)
        .addFunction("GetSkillBar", &Actor::GetSkillBar)
        .addFunction("GetSelectedObject", &Actor::_LuaGetSelectedObject)
        .addFunction("SetSelectedObject", &Actor::_LuaSetSelectedObject)
        .addFunction("UseSkill", &Actor::UseSkill)
        .addFunction("GetCurrentSkill", &Actor::GetCurrentSkill)

        .addFunction("IsUndestroyable", &Actor::IsUndestroyable)
        .addFunction("SetUndestroyable", &Actor::SetUndestroyable)
        .addFunction("GetSpeed", &Actor::GetSpeed)
        .addFunction("SetSpeed", &Actor::SetSpeed)
        .addFunction("AddSpeed", &Actor::AddSpeed)
        .addFunction("AddEffect", &Actor::_LuaAddEffect)
        .addFunction("RemoveEffect", &Actor::_LuaRemoveEffect)
        .addFunction("GetLastEffect", &Actor::_LuaGetLastEffect)

        .addFunction("GotoPosition", &Actor::_LuaGotoPosition)
        .addFunction("FollowObject", &Actor::_LuaFollowObject)
        .addFunction("GetState", &Actor::_LuaGetState)
        .addFunction("SetState", &Actor::_LuaSetState)
        .addFunction("FaceObject", &Actor::FaceObject)
        .addFunction("HeadTo", &Actor::_LuaHeadTo)
        .addFunction("IsEnemy", &Actor::IsEnemy)

        .addFunction("SetSpawnPoint", &Actor::SetSpawnPoint)
        .addFunction("GetHomePos", &Actor::_LuaGetHomePos)
        .addFunction("SetHomePos", &Actor::_LuaSetHomePos)
        .addFunction("GotoHomePos", &Actor::GotoHomePos)
        .addFunction("IsDead", &Actor::IsDead)
        .addFunction("Die", &Actor::Die)
        .addFunction("Resurrect", &Actor::Resurrect)
        .addFunction("GetActorsInRange", &Actor::_LuaGetActorsInRange)

        .addFunction("GetAttributeValue", &Actor::GetAttributeValue)
    );
}

Actor::Actor() :
    GameObject(),
    skills_(std::make_unique<SkillBar>(*this)),
    moveComp_(*this),
    autorunComp_(*this),
    collisionComp_(*this),
    resourceComp_(*this),
    attackComp_(*this),
    effectsComp_(*this),
    equipComp_(*this),
    skillsComp_(*this),
    inputComp_(*this),
    undestroyable_(false),
    retriggerTimeout_(1000)
{
    // Actor always collides
    static const Math::Vector3 CREATURTE_BB_MIN(-0.2f, 0.0f, -0.2f);
    static const Math::Vector3 CREATURTE_BB_MAX(0.2f, 1.7f, 0.2f);
    SetCollisionShape(
        std::make_unique<Math::CollisionShapeImpl<Math::BoundingBox>>(Math::ShapeTypeBoundingBox,
            CREATURTE_BB_MIN, CREATURTE_BB_MAX)
    );
    occluder_ = true;
    resourceComp_.SetMaxHealth(500);
    resourceComp_.SetMaxEnergy(50);
    resourceComp_.SetHealth(Components::SetValueType::Absolute, 500);
    resourceComp_.SetEnergy(Components::SetValueType::Absolute, 50);
}

bool Actor::SetSpawnPoint(const std::string& group)
{
    auto game = GetGame();
    if (!game)
        return false;
    auto sps = game->map_->GetSpawnPoints(group);
    auto sp = game->map_->GetFreeSpawnPoint(sps);
    if (sp.Empty())
        return false;

    transformation_.position_ = sp.position;
    transformation_.position_.y_ = game->map_->GetTerrainHeight(transformation_.position_);
    transformation_.SetYRotation(sp.rotation.EulerAngles().y_);
    return true;
}

bool Actor::GotoHomePos()
{
    if (!homePos_.Equals(transformation_.position_))
    {
        GotoPosition(homePos_);
        return true;
    }
    return false;
}

void Actor::SetSelectedObject(std::shared_ptr<GameObject> object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputComp_.Add(InputType::Select, data);
}

void Actor::GotoPosition(const Math::Vector3& pos)
{
    Utils::VariantMap data;
    data[InputDataVertexX] = pos.x_;
    data[InputDataVertexY] = pos.y_;
    data[InputDataVertexZ] = pos.z_;
    inputComp_.Add(InputType::Goto, data);
}

void Actor::FollowObject(std::shared_ptr<GameObject> object)
{
    if (object)
        FollowObject(object->id_);
    else
        FollowObject(0);
}

void Actor::FollowObject(uint32_t objectId)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = objectId;
    inputComp_.Add(InputType::Follow, data);
}

void Actor::UseSkill(uint32_t index)
{
    Utils::VariantMap data;
    data[InputDataSkillIndex] = static_cast<uint8_t>(index);
    inputComp_.Add(InputType::UseSkill, data);
}

void Actor::_LuaSetState(int state)
{
    Utils::VariantMap data;
    data[InputDataState] = static_cast<uint8_t>(state);
    inputComp_.Add(InputType::SetState, data);
}

void Actor::_LuaSetHomePos(float x, float y, float z)
{
    homePos_ = { x, y, z };
    if (Math::Equals(homePos_.y_, 0.0f))
    {
        homePos_.y_ = GetGame()->map_->GetTerrainHeight(homePos_);
    }
}

void Actor::_LuaHeadTo(float x, float y, float z)
{
    HeadTo(Math::Vector3(x, y, z));
}

std::vector<float> Actor::_LuaGetHomePos()
{
    std::vector<float> result;
    result.push_back(homePos_.x_);
    result.push_back(homePos_.y_);
    result.push_back(homePos_.z_);
    return result;
}

void Actor::_LuaFollowObject(GameObject* object)
{
    if (object)
        FollowObject(object->id_);
    else
        FollowObject(0);
}

std::vector<Actor*> Actor::_LuaGetActorsInRange(Ranges range)
{
    std::vector<Actor*> result;
    VisitInRange(range, [&](const std::shared_ptr<GameObject>& o)
    {
        AB::GameProtocol::GameObjectType t = o->GetType();
        if (t == AB::GameProtocol::ObjectTypeNpc || t == AB::GameProtocol::ObjectTypePlayer)
            result.push_back(dynamic_cast<Actor*>(o.get()));
    });
    return result;
}

void Actor::_LuaAddEffect(Actor* source, uint32_t index)
{
#ifdef DEBUG_GAME
    LOG_DEBUG << "Effect " << index << " added to " << GetName() << std::endl;
#endif
    effectsComp_.AddEffect(source ? source->GetThisDynamic<Actor>() : nullptr, index);
}

void Actor::_LuaRemoveEffect(uint32_t index)
{
    effectsComp_.RemoveEffect(index);
}

Effect* Actor::_LuaGetLastEffect(AB::Entities::EffectCategory category)
{
    auto effect = effectsComp_.GetLast(category);
    if (effect)
        return effect.get();
    return nullptr;
}

GameObject* Actor::_LuaGetSelectedObject()
{
    if (auto o = selectedObject_.lock())
        return o.get();
    return nullptr;
}

void Actor::_LuaSetSelectedObject(GameObject* object)
{
    Utils::VariantMap data;
    data[InputDataObjectId] = GetId();    // Source
    if (object)
        data[InputDataObjectId2] = object->GetId();   // Target
    else
        data[InputDataObjectId2] = 0;   // Target
    inputComp_.Add(InputType::Select, data);
}

bool Actor::Serialize(IO::PropWriteStream& stream)
{
    if (!GameObject::Serialize(stream))
        return false;
    stream.Write<uint32_t>(GetLevel());
    stream.Write<uint8_t>(GetSex());
    stream.Write<uint32_t>(GetProfIndex());
    stream.Write<uint32_t>(GetProf2Index());
    stream.Write<uint32_t>(GetModelIndex());
    const std::string skills = IO::TemplateEncoder::Encode(*skills_);
    stream.WriteString(skills);
    return true;
}

void Actor::WriteSpawnData(Net::NetworkMessage& msg)
{
    msg.Add<uint32_t>(id_);
    msg.Add<float>(transformation_.position_.x_);
    msg.Add<float>(transformation_.position_.y_);
    msg.Add<float>(transformation_.position_.z_);
    msg.Add<float>(transformation_.GetYRotation());
    msg.Add<float>(transformation_.scale_.x_);
    msg.Add<float>(transformation_.scale_.y_);
    msg.Add<float>(transformation_.scale_.z_);
    msg.Add<uint8_t>(undestroyable_ ? 1 : 0);
    msg.Add<uint8_t>(stateComp_.GetState());
    msg.Add<float>(GetSpeed());
    msg.Add<uint32_t>(GetGroupId());
    msg.Add<uint8_t>(GetGroupPos());
    IO::PropWriteStream data;
    size_t dataSize;
    Serialize(data);
    const char* cData = data.GetStream(dataSize);
    msg.AddString(std::string(cData, dataSize));
    resourceComp_.Write(msg, true);
    effectsComp_.Write(msg);
}

void Actor::OnEndUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState() && !stateComp_.IsDead())
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
}

void Actor::OnStartUseSkill(Skill* skill)
{
    // These change the state
    if (skill->IsChangingState())
        stateComp_.SetState(AB::GameProtocol::CreatureStateUsingSkill);
}

void Actor::HeadTo(const Math::Vector3& pos)
{
    moveComp_.HeadTo(pos);
}

void Actor::FaceObject(GameObject* object)
{
    if (object)
        HeadTo(object->transformation_.position_);
}

Skill* Actor::GetCurrentSkill() const
{
    return skills_->GetCurrentSkill();
}

void Actor::_LuaGotoPosition(float x, float y, float z)
{
    Math::Vector3 pos(x, y, z);
    if (Math::Equals(y, 0.0f))
    {
        pos.y_ = GetGame()->map_->GetTerrainHeight(pos);
    }
    GotoPosition(pos);
}

int Actor::_LuaGetState()
{
    return static_cast<int>(stateComp_.GetState());
}

void Actor::UpdateRanges()
{
    ranges_.clear();
    std::vector<GameObject*> res;

    static const float RANGE_AGGRO = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeAggro];
    static const float RANGE_COMPASS = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeCompass];
    static const float RANGE_SPIRIT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeSpirit];
    static const float RANGE_EARSHOT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeEarshot];
    static const float RANGE_CASTING = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeCasting];
    static const float RANGE_PROJECTILE = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeProjectile];
    static const float RANGE_HALF = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeHalf];
    static const float RANGE_TOUCH = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeTouch];
    static const float RANGE_ADJECENT = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeAdjecent];

    static const float RANGE_VISIBLE = (*GetSubsystem<ConfigManager>())[ConfigManager::Key::RangeVisible];

    // Compass radius
    if (QueryObjects(res, RANGE_COMPASS))
    {
        for (const auto& o : res)
        {
            if (o != this && o->GetType() > AB::GameProtocol::ObjectTypeSentToPlayer)
            {
                auto so = o->shared_from_this();
                Math::Vector3 objectPos = o->GetPosition();
                Math::Vector3 myPos = GetPosition();
                if (myPos.Distance(objectPos) <= RANGE_AGGRO)
                    ranges_[Ranges::Aggro].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_COMPASS)
                    ranges_[Ranges::Compass].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_SPIRIT)
                    ranges_[Ranges::Spirit].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_EARSHOT)
                    ranges_[Ranges::Earshot].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_CASTING)
                    ranges_[Ranges::Casting].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_PROJECTILE)
                    ranges_[Ranges::Projectile].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_HALF)
                    ranges_[Ranges::HalfCompass].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_TOUCH)
                    ranges_[Ranges::Touch].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_ADJECENT)
                    ranges_[Ranges::Adjecent].push_back(so);
                if (myPos.Distance(objectPos) <= RANGE_VISIBLE)
                    ranges_[Ranges::Visible].push_back(so);
            }
        }
    }
}

void Actor::Update(uint32_t timeElapsed, Net::NetworkMessage& message)
{
    GameObject::Update(timeElapsed, message);
    UpdateRanges();

    stateComp_.Update(timeElapsed);
    resourceComp_.Update(timeElapsed);
    inputComp_.Update(timeElapsed, message);

    attackComp_.Update(timeElapsed);
    skillsComp_.Update(timeElapsed);
    effectsComp_.Update(timeElapsed);
    moveComp_.Update(timeElapsed);
    autorunComp_.Update(timeElapsed);

    stateComp_.Write(message);
    moveComp_.Write(message);

    skillsComp_.Write(message);
    effectsComp_.Write(message);
    resourceComp_.Write(message);
}

bool Actor::Die()
{
    if (!IsDead())
    {
        stateComp_.SetState(AB::GameProtocol::CreatureStateDead);
        resourceComp_.SetHealth(Components::SetValueType::Absolute, 0);
        resourceComp_.SetEnergy(Components::SetValueType::Absolute, 0);
        resourceComp_.SetAdrenaline(Components::SetValueType::Absolute, 0);
        autorunComp_.autoRun_ = false;
        return true;
    }
    return false;
}

bool Actor::Resurrect(int16_t precentHealth, int16_t percentEnergy)
{
    if (IsDead())
    {
        int16_t health = (resourceComp_.GetMaxHealth() / 100) * precentHealth;
        resourceComp_.SetHealth(Components::SetValueType::Absolute, health);
        int16_t energy = (resourceComp_.GetMaxEnergy() / 100) * percentEnergy;
        resourceComp_.SetEnergy(Components::SetValueType::Absolute, energy);
        stateComp_.SetState(AB::GameProtocol::CreatureStateIdle);
        return true;
    }
    return false;
}

bool Actor::IsEnemy(Actor* other)
{
    if (GetGroupId() == other->GetGroupId())
        return false;
    // TODO: What if different groups are allies
    return true;
}

}
