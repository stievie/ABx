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


#include "Actor.h"
#include <abshared/AttribAlgos.h>
#include "EffectsComp.h"
#include "SkillBar.h"
#include "SkillManager.h"
#include <abscommon/DataClient.h>
#include <abshared/TemplEncoder.h>
#include <abshared/SkillsHelper.h>

namespace Game {

void SkillBar::RegisterLua(kaguya::State& state)
{
    state["SkillBar"].setClass(kaguya::UserdataMetatable<SkillBar>()
        .addFunction("GetSkill", &SkillBar::_LuaGetSkill)
        .addFunction("SetSkill", &SkillBar::_LuaSetSkill)
        .addFunction("RemoveSkill", &SkillBar::RemoveSkill)
        .addFunction("GetCurrentSkill", &SkillBar::GetCurrentSkill)
        .addFunction("UseSkill", &SkillBar::UseSkill)
        .addFunction("GetSkillsWithEffect", &SkillBar::_LuaGetSkillsWithEffect)
        .addFunction("GetSkillsWithTarget", &SkillBar::_LuaGetSkillsWithTarget)
        .addFunction("GetSkillsWithEffectTarget", &SkillBar::_LuaGetSkillsWithEffectTarget)
        .addFunction("AddSkill", &SkillBar::_LuaAddSkill)
        .addFunction("HaveAttribute", &SkillBar::HaveAttribute)
        .addFunction("SetAttributeRank", &SkillBar::SetAttributeRank)
        .addFunction("GetAttributeRank", &SkillBar::GetAttributeRank)
        .addFunction("Load", &SkillBar::Load)
    );
}

Skill* SkillBar::_LuaGetSkill(int pos)
{
    if (pos < 0)
        return nullptr;
    auto s = GetSkill(pos);
    if (s)
        return s.get();
    return nullptr;
}

int SkillBar::_LuaAddSkill(uint32_t skillIndex)
{
    SkillManager* sm = GetSubsystem<SkillManager>();
    ea::shared_ptr<Skill> skill = sm->Get(skillIndex);
    if (!skill)
        return -1;

    size_t i = 0;
    for (const auto& s : skills_)
    {
        if (!s)
        {
            // Use first free slot
            skills_[i] = skill;
            return static_cast<int>(i);
        }
        ++i;
    }
    return -1;
}

bool SkillBar::_LuaSetSkill(int pos, uint32_t skillIndex)
{
    auto* sm = GetSubsystem<SkillManager>();
    auto skill = sm->Get(skillIndex);
    if (skill)
    {
        if (!SetSkill(static_cast<int>(pos), skill))
            return false;
    }
    else
        LOG_WARNING << "No skill with index " << skillIndex << " found" << std::endl;

    const uint32_t newIndex = GetIndexOfSkill(static_cast<int>(pos));

    return skillIndex == newIndex;
}

AB::GameProtocol::SkillError SkillBar::UseSkill(int index, ea::shared_ptr<Actor> target)
{
    if (index < 0 || index >= PLAYER_MAX_SKILLS)
        return AB::GameProtocol::SkillError::InvalidSkill;
    Skill* s = skills_[static_cast<size_t>(index)].get();
    if (!s)
        return AB::GameProtocol::SkillError::InvalidSkill;

    if (s->IsChangingState())
    {
        if (owner_.IsKnockedDown())
        {
            // Can only use skills like Stances, Shouts when knocked down
            return AB::GameProtocol::SkillError::CannotUseSkill;
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
    const SkillsArray& _skills = GetArray();
    SkillIndices sis;
    size_t i = 0;
    for (const auto& s : _skills)
    {
        sis[i] = (s ? s->data_.index : 0);
        ++i;
    }

    return IO::SkillTemplateEncode(prof1_, prof2_, GetAttributes(), sis);
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

void SkillBar::SetAttributes(const Attributes& attributes)
{
    for (const auto& a : attributes)
    {
        if (a.index == Attribute::None)
            continue;
        SetAttributeRank(a.index, a.value);
    }
}

void SkillBar::ResetAttributes()
{
    attributes_.fill({ Attribute::None, 0u });
}

void SkillBar::InitAttributes()
{
    ResetAttributes();
    size_t i = 0;
    for (const auto& a : prof1_.attributes)
    {
        attributes_[i].index = static_cast<Attribute>(a.index);
        ++i;
    }
    for (const auto& a : prof2_.attributes)
    {
        if (!a.primary)
        {
            // Can not have a rank of the primary attribute of the 2nd profession
            attributes_[i].index = static_cast<Attribute>(a.index);
            ++i;
        }
    }
}

bool SkillBar::Load(const std::string& str, bool locked)
{
    AB::Entities::Profession p1;
    AB::Entities::Profession p2;
    Attributes attribs;
    std::array<uint32_t, PLAYER_MAX_SKILLS> skills;
    if (!IO::SkillTemplateDecode(str, p1, p2, attribs, skills))
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
    auto* skillMan = GetSubsystem<SkillManager>();

    auto professionsMatch = [&](const AB::Entities::Skill& skill)
    {
        bool result = SkillProfessionMatches(skill, prof1_, &prof2_);
#if 0
        if (!result)
        {
            LOG_INFO << "Professions do not match " << skill.name << ", " << skill.professionUuid;
            LOG_INFO << "  Prof1 " << prof1_.uuid;
            LOG_INFO << "  Prof2 " << prof2_.uuid;
            LOG_INFO << std::endl;
        }
#endif
        return result;
    };

    auto hasAccess = [&](const AB::Entities::Skill& skill)
    {
        if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessPlayer))
            return true;
        if (locked)
        {
            // This player can have GM locked skills
            if (AB::Entities::HasSkillAccess(skill, AB::Entities::SkillAccessGM))
                return true;
        }
        LOG_INFO << "No access to " << skill.name << std::endl;
        return false;
    };

    for (size_t i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        skills_[i] = skillMan->Get(skills[i]);
        if (skills_[i] && (!hasAccess(skills_[i]->data_) || !professionsMatch(skills_[i]->data_)))
            // This player can not have locked skills
            skills_[i] = skillMan->Get(0);
    }

    return true;
}

bool SkillBar::SetSecondaryProfession(uint32_t index)
{
    if (prof2_.index == index)
        return true;

    // index = 0 means no secondary profession, which is legit
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
    InitProf2Attribs(attributes_, prof1_, (index != 0) ? &prof2_ : nullptr);
    return true;
}

void SkillBar::ResetSkills()
{
    for (size_t i = 0; i < PLAYER_MAX_SKILLS; i++)
    {
        skills_[i].reset();
    }
}

ea::shared_ptr<Skill> SkillBar::GetSkill(int pos)
{
    if (pos < 0)
        return ea::shared_ptr<Skill>();
    if (pos < PLAYER_MAX_SKILLS)
        return skills_[static_cast<size_t>(pos)];
    return ea::shared_ptr<Skill>();
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

bool SkillBar::SetSkill(int pos, ea::shared_ptr<Skill> skill)
{
    if (pos < 0 || pos >= PLAYER_MAX_SKILLS)
        return false;
    skills_[static_cast<size_t>(pos)] = skill;
    return true;
}

int SkillBar::GetUsedAttributePoints() const
{
    return GetUsedAttribPoints(attributes_);
}

int SkillBar::GetAvailableAttributePoints() const
{
    return static_cast<int>(owner_.GetAttributePoints()) - GetUsedAttributePoints();
}

const AttributeValue* SkillBar::GetAttribute(Attribute index) const
{
    for (const auto& a : attributes_)
    {
        if (a.index == index)
            return &a;
    }
    return nullptr;
}

bool SkillBar::SetAttributeRank(Attribute index, uint32_t value)
{
    // This works only when professions are set, which fill the attributes array
    auto it = std::find_if(attributes_.begin(), attributes_.end(), [&](const AttributeValue& attrib) {
        return attrib.index == index;
    });
    if (it == attributes_.end())
        return false;
    if ((*it).value == value)
        return true;
    if (value > (*it).value)
    {
        // If increasing the rank see if there are enough free points available
        const int cost = CalcAttributeCost(static_cast<int>(value));
        const int used = GetUsedAttribPoints(attributes_, static_cast<int>(index));
        const int total = owner_.GetAttributePoints();
        const int avail = total - used;
        if (cost > avail)
        {
            LOG_WARNING << "Not enough attribute points available. Required: " << cost <<
                ", available: " << avail << ", total: " << owner_.GetAttributePoints() << std::endl;
            return false;
        }
    }
    (*it).value = value;
    return true;
}

uint32_t SkillBar::GetAttributeRank(Attribute index) const
{
    // This works only when professions are set, which fill the attributes array
    return GetAttribRank(attributes_, index);
}

ea::vector<uint32_t> SkillBar::GetSkillsWithEffect(SkillEffect effect, bool rechargedOnly /* = false */) const
{
    ea::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasEffect(effect) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

ea::vector<uint32_t> SkillBar::GetSkillsWithTarget(SkillEffectTarget target, bool rechargedOnly /* = false */) const
{
    ea::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasTarget(target) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

ea::vector<uint32_t> SkillBar::GetSkillsWithEffectTarget(SkillEffect effect, SkillEffectTarget target, bool rechargedOnly /* = false */) const
{
    ea::vector<uint32_t> result;
    uint32_t i = 0;
    for (const auto& skill : skills_)
    {
        if (skill && skill->HasEffect(effect) && skill->HasTarget(target) && (!rechargedOnly || skill->IsRecharged()))
            result.push_back(i);
        ++i;
    }
    return result;
}

std::string SkillBar::GetClasses() const
{
    if (prof1_.index == 0)
        return "";

    std::string result = prof1_.abbr;
    if (prof2_.index != 0)
        result += "/" + prof2_.abbr;

    return result;
}

}
