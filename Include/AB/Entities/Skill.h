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

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_SKILLS = "game_skills";

/// Skill type Tree
/// https://wiki.guildwars.com/wiki/Skill_type
enum SkillType : uint64_t
{
    SkillTypeSkill = 0ll,
    SkillTypeAttack = 1ll,
        SkillTypeRangedAttack        = SkillTypeAttack | 1ll << 16,
            SkillTypeBowAttack           = SkillTypeRangedAttack | 1ll << 32,
            SkillTypeSpearAttack         = SkillTypeRangedAttack | 2ll << 32,
        SkillTypeMeleeAttack         = SkillTypeAttack | 2ll << 16,
            SkillTypeAxeAttack           = SkillTypeMeleeAttack | 1ll << 32,
            SkillTypeDaggerAttack        = SkillTypeMeleeAttack | 2ll << 32,
                SkillTypeLeadAttack          = SkillTypeDaggerAttack | 1ll << 48,
                SkillTypeOffHandAttack       = SkillTypeDaggerAttack | 2ll << 48,
                SkillTypeDualAttack          = SkillTypeDaggerAttack | 4ll << 48,
            SkillTypeHammerAttack        = SkillTypeMeleeAttack | 4ll << 16,
            SkillTypeScyteAttack         = SkillTypeMeleeAttack | 8ll << 16,
            SkillTypeSwordAttack         = SkillTypeMeleeAttack | 16ll << 16,
        SkillTypePetAttack           = SkillTypeAttack | 4ll << 16,
    SkillTypeShout = 2ll,
    SkillTypeChant = 4ll,
    SkillTypeEcho = 8ll,
    SkillTypeForm = 16ll,
    SkillTypeGlypthe = 32ll,
    SkillTypePreparation = 64ll,
    SkillTypeRitual = 128ll,
        SkillTypeBindingRitual       = SkillTypeRitual | 1ll << 16,
        SkillTypeNatureRitual        = SkillTypeRitual | 2ll << 16,
    SkillTypeSignet = 256ll,
    SkillTypeSpell = 512ll,
        SkillTypeEnchantment         = SkillTypeSpell | 1ll << 16,
            SkillTypeFlashEnchantment    = SkillTypeEnchantment | 1ll << 32,
        SkillTypeHex                 = SkillTypeSpell | 2ll << 16,
        SkillTypeItemSpell           = SkillTypeSpell | 4ll << 16,
        SkillTypeWardSpell           = SkillTypeSpell | 8ll << 16,
        SkillTypeWeaponSpell         = SkillTypeSpell | 16ll << 16,
        SkillTypeWellSpell           = SkillTypeSpell | 32ll << 16,
    SkillTypeStance = 1024ll,
    SkillTypeTrap = 2048ll,
    SkillTypeAll = 0xFFFFFFFFFFFFFFFF
};

enum SkillAccess : uint32_t
{
    SkillAccessNone = 0,
    SkillAccessPlayer = 1,
    SkillAccessGM = 1 << 1,
    SkillAccessMonster = 1 << 2
};

struct Skill : Entity
{
    static constexpr const char* KEY()
    {
        return KEY_SKILLS;
    }
    template<typename S>
    void serialize(S& s)
    {
        s.ext(*this, BaseClass<Entity>{});
        s.value4b(index);
        s.text1b(name, Limits::MAX_SKILL_NAME);
        s.text1b(attributeUuid, Limits::MAX_UUID);
        s.text1b(professionUuid, Limits::MAX_UUID);
        s.value8b(type);
        s.value1b(isElite);
        s.text1b(description, Limits::MAX_SKILL_DESCRIPTION);
        s.text1b(shortDescription, Limits::MAX_SKILL_SHORT_DESCRIPTION);
        s.text1b(icon, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.value4b(access);
        s.text1b(soundEffect, Limits::MAX_FILENAME);
        s.text1b(particleEffect, Limits::MAX_FILENAME);
        s.value4b(activation);
        s.value4b(recharge);
        s.value4b(costEnergy);
        s.value4b(costEnergyRegen);
        s.value4b(costAdrenaline);
        s.value4b(costOvercast);
        s.value4b(costHp);
    }

    uint32_t index = 0;
    std::string name;
    std::string attributeUuid{ EMPTY_GUID };
    std::string professionUuid{ EMPTY_GUID };
    SkillType type{ SkillTypeSkill };
    bool isElite{ false };
    std::string description;
    std::string shortDescription;
    std::string icon;
    std::string script;
    uint32_t access{ SkillAccessNone };
    std::string soundEffect;
    std::string particleEffect;
    int32_t activation{ 0 };
    int32_t recharge{ 0 };
    int32_t costEnergy{ 0 };
    int32_t costEnergyRegen{ 0 };
    int32_t costAdrenaline{ 0 };
    int32_t costOvercast{ 0 };
    int32_t costHp{ 0 };
};

inline bool HasSkillAccess(const Skill& skill, SkillAccess access)
{
    return (skill.access & access) == access;
}

}
}
