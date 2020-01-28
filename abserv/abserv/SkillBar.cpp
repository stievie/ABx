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
#include "AttribAlgos.h"

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

Skill* SkillBar::_LuaGetSkill(int index)
{
    if (index < 0)
        return nullptr;
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

    size_t i = 0;
    for (const auto& s : skills_)
    {
        if (!s)
        {
            skills_[i] = skill;
            return static_cast<int>(i);
        }
        ++i;
    }
    return -1;
}

AB::GameProtocol::SkillError SkillBar::UseSkill(int index, std::shared_ptr<Actor> target)
{
    if (index < 0 || index >= PLAYER_MAX_SKILLS)
        return AB::GameProtocol::SkillErrorInvalidSkill;
    Skill* s = skills_[static_cast<size_t>(index)].get();
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

    currentSkillIndex_ = index;
    return s->StartUse(owner_.GetPtr<Actor>(), target); // overcast
}

Skill* SkillBar::GetCurrentSkill() const
{
    if (currentSkillIndex_ > -1 && currentSkillIndex_ < PLAYER_MAX_SKILLS)
    {
        Skill* skill = skills_[static_cast<size_t>(currentSkillIndex_)].get();
        if (skill && skill->IsUsing())
            return skill;
    }
    return nullptr;
}

void SkillBar::Update(uint32_t timeElapsed)
{
    for (size_t i = 0; i < PLAYER_MAX_SKILLS; ++i)
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
    if (std::find_if(prof1_.attributes.begin(), prof1_.attributes.end(), [&](const AB::Entities::AttriInfo& i)
    {
        return i.index == index;
    }) != prof1_.attributes.end())
        return true;
    if (std::find_if(prof2_.attributes.begin(), prof2_.attributes.end(), [&](const AB::Entities::AttriInfo& i)
    {
        return i.index == index;
    }) != prof2_.attributes.end())
        return true;
    return false;
}

void SkillBar::SetAttributes(const AB::Attributes& attributes)
{
    for (const auto& a : attributes)
    {
        if (a.index == 0)
            continue;
        SetAttributeValue(a.index, a.value);
    }
}

void SkillBar::ResetAttributes()
{
    attributes_.fill({ 99u, 0u });
}

void SkillBar::ResetSecondProfAttributes()
{
    // Clear all attributed exceptt for primary profession
    for (size_t i = prof1_.attributeCount; i < PLAYER_MAX_ATTRIBUTES; ++i)
    {
        attributes_[i].index = 99;
        attributes_[i].value = 0;
    }
}

void SkillBar::InitAttributes()
{
    ResetAttributes();
    size_t i = 0;
    for (const auto& a : prof1_.attributes)
    {
        attributes_[i].index = a.index;
        ++i;
    }
    for (const auto& a : prof2_.attributes)
    {
        if (!a.primary)
        {
            attributes_[i].index = a.index;
            ++i;
        }
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
    prof2_.attributes.clear();

    auto dataClient = GetSubsystem<IO::DataClient>();
    if (p2.index != 0)
    {
        if (!dataClient->Read(prof2_))
        {
            LOG_WARNING << "Error loading secondary profession with index " << prof2_.index << std::endl;
        }
    }
    InitAttributes();
    SetAttributes(attribs);
    auto skillMan = GetSubsystem<SkillManager>();
    for (size_t i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        skills_[i] = skillMan->Get(skills[i]);
        if (skills_[i] && skills_[i]->data_.isLocked && !locked)
            // This player can not have locked skills
            skills_[i] = skillMan->Get(0);
    }

    return true;
}

bool SkillBar::SetSecondaryProfession(uint32_t index)
{
    AB::Entities::Profession p2;
    p2.uuid = Utils::Uuid::EMPTY_UUID;
    p2.index = index;

    auto dataClient = GetSubsystem<IO::DataClient>();
    if (index != 0)
    {
        if (!dataClient->Read(p2))
        {
            LOG_WARNING << "Error loading secondary profession with index " << prof2_.index << std::endl;
            return false;
        }
    }
    prof2_ = p2;
    ResetSecondProfAttributes();
    return true;
}

std::shared_ptr<Skill> SkillBar::GetSkill(int pos)
{
    if (pos < 0)
        return std::shared_ptr<Skill>();
    if (pos < PLAYER_MAX_SKILLS)
        return skills_[static_cast<size_t>(pos)];
    return std::shared_ptr<Skill>();
}

uint32_t SkillBar::GetIndexOfSkill(int pos)
{
    if (pos < 0)
        return 0;
    if (pos >= PLAYER_MAX_SKILLS)
        return 0;
    auto skill = skills_[static_cast<size_t>(pos)];
    if (!skill)
        return 0;
    return skill->GetIndex();
}

bool SkillBar::SetSkillByIndex(int pos, uint32_t skillIndex)
{
    auto* sm = GetSubsystem<SkillManager>();
    auto skill = sm->Get(skillIndex);
    if (!skill)
        return false;
    return SetSkill(pos, skill);
}

int SkillBar::GetUsedAttributePoints() const
{
    int result = 0;
    for (const auto& a : attributes_)
        if (a.value > 0)
            result += AB::CalcAttributeCost(static_cast<int>(a.value));
    return result;
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
    // This works only when professions are set, which fill the attributes array
    auto it = std::find_if(attributes_.begin(), attributes_.end(), [&](const AB::AttributeValue& attrib) {
        return attrib.index == index;
    });
    if (it == attributes_.end())
        return false;
    if ((*it).value == value)
        return false;
    if (value > (*it).value)
    {
        int cost = AB::CalcAttributeCost(static_cast<int>(value));
        if (cost > (static_cast<int>(owner_.GetAttributePoints()) - GetUsedAttributePoints()))
            return false;
    }
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

std::vector<uint32_t> SkillBar::GetSkillsWithTarget(SkillEffectTarget target, bool rechargedOnly /* = false */) const
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

std::vector<uint32_t> SkillBar::GetSkillsWithEffectTarget(SkillEffect effect, SkillEffectTarget target, bool rechargedOnly /* = false */) const
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
