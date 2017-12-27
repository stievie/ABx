#pragma once

#include "Utils.h"

namespace Game {

class Creature;

enum SkillType : uint32_t
{
    SkillTypeUnknown = 0,
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

class Skill : public std::enable_shared_from_this<Skill>
{
private:
    kaguya::State luaState_;
    int64_t startUse_;
    int64_t recharged_;
    void InitializeLua();
    Creature* source_;
    Creature* target_;
public:
    static void RegisterLua(kaguya::State& state);

    explicit Skill(uint32_t id) :
        id_(id),
        startUse_(0),
        recharged_(0),
        source_(nullptr),
        target_(nullptr),
        type_(SkillTypeUnknown),
        energy_(0),
        adrenaline_(0),
        activation_(0),
        recharge_(0),
        overcast_(0)
    {
        InitializeLua();
    }
    ~Skill() = default;

    bool LoadScript(const std::string& fileName);
    void Update(uint32_t timeElapsed);

    bool StartUse(Creature* source, Creature* target);
    void CancelUse();

    bool IsUsing() const { return startUse_ != 0; }
    bool IsRecharged() const { return recharged_ <= Utils::AbTick(); }
    bool IsType(SkillType type)
    {
        return (type_ & type) == type;
    }

    uint32_t id_;

    SkillType type_;
    bool elite_;
    std::string name_;
    uint32_t energy_;
    uint32_t adrenaline_;
    uint32_t activation_;
    uint32_t recharge_;
    uint32_t overcast_;
};

}
