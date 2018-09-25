#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_SKILLS = "game_skills";

enum SkillType : uint32_t
{
    SkillTypeSkill = 0,
    SkillTypeAttack = 1,
        SkillTypeRangedAttack        = SkillTypeAttack | 1 << 8,
            SkillTypeBowAttack           = SkillTypeRangedAttack | 1 << 16,
            SkillTypeSpearAttack         = SkillTypeRangedAttack | 2 << 16,
        SkillTypeMeleeAttack         = SkillTypeAttack | 2 << 8,
            SkillTypeAxeAttack           = SkillTypeMeleeAttack | 1 << 16,
            SkillTypeDaggerAttack        = SkillTypeMeleeAttack | 2 << 16,
                SkillTypeLeadAttack          = SkillTypeDaggerAttack | 1 << 24,
                SkillTypeOffHandAttack       = SkillTypeDaggerAttack | 2 << 24,
                SkillTypeDualAttack          = SkillTypeDaggerAttack | 3 << 24,
            SkillTypeHammerAttack        = SkillTypeMeleeAttack | 3 << 16,
            SkillTypeScyteAttack         = SkillTypeMeleeAttack | 4 << 16,
            SkillTypeSwordAttack         = SkillTypeMeleeAttack | 5 << 16,
        SkillTypePetAttack           = SkillTypeAttack | 3 << 8,
    SkillTypeShout = 2,
    SkillTypeChant = 3,
    SkillTypeEcho = 4,
    SkillTypeForm = 5,
    SkillTypeGlypthe = 6,
    SkillTypePreparation = 7,
    SkillTypeRitual = 8,
        SkillTypeBindingRitual       = SkillTypeRitual | 1 << 8,
        SkillTypeNatureRitual        = SkillTypeRitual | 2 << 8,
    SkillTypeSignet = 9,
    SkillTypeSpell = 10,
        SkillTypeEnchantment         = SkillTypeSpell | 1 << 8,
            SkillTypeFlashEnchantment    = SkillTypeEnchantment | 1 << 16,
        SkillTypeHex                 = SkillTypeSpell | 2 << 8,
        SkillTypeItemSpell           = SkillTypeSpell | 3 << 8,
        SkillTypeWardSpell           = SkillTypeSpell | 4 << 8,
        SkillTypeWeaponSpell         = SkillTypeSpell | 5 << 8,
        SkillTypeWellSpell           = SkillTypeSpell | 6 << 8,
    SkillTypeStance = 11,
    SkillTypeTrap = 12
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
        s.value4b(type);
        s.value1b(isElite);
        s.text1b(description, Limits::MAX_SKILL_DESCRIPTION);
        s.text1b(shortDescription, Limits::MAX_SKILL_SHORT_DESCRIPTION);
        s.text1b(icon, Limits::MAX_FILENAME);
        s.text1b(script, Limits::MAX_FILENAME);
        s.value1b(isLocked);
    }

    uint32_t index = 0;
    std::string name;
    std::string attributeUuid = EMPTY_GUID;
    SkillType type = SkillTypeSkill;
    bool isElite = false;
    std::string description;
    std::string shortDescription;
    std::string icon;
    std::string script;
    bool isLocked = false;
};

}
}
