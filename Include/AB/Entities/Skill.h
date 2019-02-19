#pragma once

#include <AB/Entities/Entity.h>
#include <AB/Entities/Limits.h>

namespace AB {
namespace Entities {

static constexpr auto KEY_SKILLS = "game_skills";

/// Skill type Tree
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
    SkillTypeTrap = 2048ll
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
        s.value1b(isLocked);
        s.text1b(soundEffect, Limits::MAX_FILENAME);
        s.text1b(particleEffect, Limits::MAX_FILENAME);
    }

    uint32_t index = 0;
    std::string name;
    std::string attributeUuid = EMPTY_GUID;
    std::string professionUuid = EMPTY_GUID;
    SkillType type = SkillTypeSkill;
    bool isElite = false;
    std::string description;
    std::string shortDescription;
    std::string icon;
    std::string script;
    bool isLocked = false;
    std::string soundEffect;
    std::string particleEffect;
};

}
}
