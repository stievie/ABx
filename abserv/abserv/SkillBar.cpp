#include "stdafx.h"
#include "SkillBar.h"
#include "TemplateEncoder.h"
#include "Subsystems.h"
#include "DataClient.h"
#include "UuidUtils.h"
#include "SkillManager.h"
#include "Subsystems.h"
#include "Actor.h"
#include "EffectsComp.h"

namespace Game {

void SkillBar::RegisterLua(kaguya::State& state)
{
    state["SkillBar"].setClass(kaguya::UserdataMetatable<SkillBar>()
        .addFunction("GetSkill", &SkillBar::_LuaGetSkill)
        .addFunction("GetCurrentSkill", &SkillBar::GetCurrentSkill)
        .addFunction("UseSkill", &SkillBar::UseSkill)
        .addFunction("GetSkillsWithEffect", &SkillBar::_LuaGetSkillsWithEffect)
        .addFunction("GetSkillsWithTarget", &SkillBar::_LuaGetSkillsWithTarget)
        .addFunction("GetSkillsWithEffectTarget", &SkillBar::_LuaGetSkillsWithEffectTarget)
        .addFunction("AddSkill", &SkillBar::_LuaAddSkill)
        .addFunction("HaveAttribute", &SkillBar::HaveAttribute)
        .addFunction("SetAttributeValue", &SkillBar::SetAttributeValue)
        .addFunction("GetAttributeValue", &SkillBar::GetAttributeValue)
        .addFunction("Load", &SkillBar::Load)
    );
}

Skill* SkillBar::_LuaGetSkill(uint32_t index)
{
    auto s = GetSkill(index);
    if (s)
        return s.get();
    return nullptr;
}

int SkillBar::_LuaAddSkill(uint32_t skillIndex)
{
    SkillManager* sm = GetSubsystem<SkillManager>();
    std::shared_ptr<Skill> skill = sm->Get(skillIndex);
    if (!skill)
        return -1;

    int i = 0;
    for (const auto& s : skills_)
    {
        if (!s)
        {
            skills_[i] = skill;
            return i;
        }
        ++i;
    }
    return -1;
}

AB::GameProtocol::SkillError SkillBar::UseSkill(int index, std::shared_ptr<Actor> target)
{
    if (index < 0 || index >= PLAYER_MAX_SKILLS)
        return AB::GameProtocol::SkillErrorInvalidSkill;
    Skill* s = skills_[index].get();
    if (!s)
        return AB::GameProtocol::SkillErrorInvalidSkill;

    if (s->IsChangingState())
    {
        if (owner_.IsKnockedDown())
        {
            // Can only use skills like Stances, Shouts when knocked down
            return AB::GameProtocol::SkillErrorCannotUseSkill;
        }
        // If changing state cancel old skill. Only one changing state skill at a time.
        Skill* oldSkill = GetCurrentSkill();
        if (oldSkill && oldSkill->IsUsing())
            oldSkill->CancelUse();
    }

    return s->StartUse(owner_.GetThis<Actor>(), target); // overcast
}

Skill* SkillBar::GetCurrentSkill() const
{
    if (currentSkillIndex_ > -1 && currentSkillIndex_ < PLAYER_MAX_SKILLS)
    {
        Skill* skill = skills_[currentSkillIndex_].get();
        if (skill && skill->IsUsing())
            return skill;
    }
    return nullptr;
}

void SkillBar::Update(uint32_t timeElapsed)
{
    for (int i = 0; i < PLAYER_MAX_SKILLS; ++i)
    {
        if (skills_[i])
            skills_[i]->Update(timeElapsed);
    }
}

std::string SkillBar::Encode()
{
    return IO::TemplateEncoder::Encode(*this);
}

bool SkillBar::HaveAttribute(uint32_t index)
{
    if (std::find_if(prof1_.attributeIndices.begin(), prof1_.attributeIndices.end(), [&](uint32_t i)
    {
        return i == index;
    }) != prof1_.attributeIndices.end())
        return true;
    if (std::find_if(prof2_.attributeIndices.begin(), prof2_.attributeIndices.end(), [&](uint32_t i)
    {
        return i == index;
    }) != prof2_.attributeIndices.end())
        return true;
    return false;
}

void SkillBar::SetAttributes(const AB::Attributes& attributes)
{
    for (const auto& a : attributes)
    {
        if (a.index == 0)
            continue;
        if (!SetAttributeValue(a.index, a.value))
        {
            LOG_ERROR << "Error setting attribute " << a.index << " to value " << a.value << std::endl;
        }
    }
}

void SkillBar::ResetAttributes()
{
    attributes_.fill({ 99u, 0u });
}

void SkillBar::InitAttributes()
{
    ResetAttributes();
    int i = 0;
    for (uint32_t index : prof1_.attributeIndices)
    {
        attributes_[i].index = index;
        ++i;
    }
    for (uint32_t index : prof2_.attributeIndices)
    {
        attributes_[i].index = index;
        ++i;
    }
}

bool SkillBar::Load(const std::string& str, bool locked)
{
    AB::Entities::Profession p1;
    AB::Entities::Profession p2;
    AB::Attributes attribs;
    std::array<uint32_t, PLAYER_MAX_SKILLS> skills;
    if (!IO::TemplateEncoder::Decode(str, p1, p2, attribs, skills))
        return false;

    prof2_.uuid = Utils::Uuid::EMPTY_UUID;
    prof2_.index = p2.index;
    prof2_.attributeUuids.clear();
    prof2_.attributeCount = 0;
    prof2_.attributeIndices.clear();

    auto dataClient = GetSubsystem<IO::DataClient>();
    if (p2.index != AB::Entities::INVALID_INDEX)
    {
        if (!dataClient->Read(prof2_))
        {
            LOG_WARNING << "Error loading secondary profession with index " << prof2_.index << std::endl;
        }
    }
    InitAttributes();
    SetAttributes(attribs);
    auto skillMan = GetSubsystem<SkillManager>();
    for (int i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        skills_[i] = skillMan->Get(skills[i]);
        if (skills_[i] && skills_[i]->data_.isLocked && !locked)
            // This player can not have locked skills
            skills_[i] = skillMan->Get(0);
    }

    return true;
}

std::shared_ptr<Skill> SkillBar::GetSkill(uint32_t index)
{
    if (index < PLAYER_MAX_SKILLS)
        return skills_[index];
    return std::shared_ptr<Skill>();
}

const AB::AttributeValue* SkillBar::GetAttribute(uint32_t index) const
{
    for (const auto& a : attributes_)
    {
        if (a.index == index)
            return &a;
    }
    return nullptr;
}

bool SkillBar::SetAttributeValue(uint32_t index, uint32_t value)
{
    // This works only when professions are set, shich fill the attributes array
    auto it = std::find_if(attributes_.begin(), attributes_.end(), [&](const AB::AttributeValue& attrib) {
        return attrib.index == index;
    });
    if (it == attributes_.end())
        return false;
    (*it).value = value;
    return true;
}

uint32_t SkillBar::GetAttributeValue(uint32_t index) const
{
    // This works only when professions are set, shich fill the attributes array
    auto it = std::find_if(attributes_.begin(), attributes_.end(), [&](const AB::AttributeValue& attrib)
    {
        return attrib.index == index;
    });
    if (it == attributes_.end())
        return 0;
    return (*it).value;
}

std::vector<uint32_t> SkillBar::GetSkillsWithEffect(SkillEffect effect, bool rechargedOnly /* = false */) const
{
    std::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasEffect(effect) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

std::vector<uint32_t> SkillBar::GetSkillsWithTarget(SkillTarget target, bool rechargedOnly /* = false */) const
{
    std::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasTarget(target) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

std::vector<uint32_t> SkillBar::GetSkillsWithEffectTarget(SkillEffect effect, SkillTarget target, bool rechargedOnly /* = false */) const
{
    std::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasEffect(effect) && skill->HasTarget(target) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

}
