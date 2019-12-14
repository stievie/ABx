MAX_SKILLS = 8

-- Skill type tree
SkillTypeSkill = 0          -- Everything is a skill
SkillTypeAttack = 1
    SkillTypeRangedAttack        = SkillTypeAttack | 1 << 16
        SkillTypeBowAttack           = SkillTypeRangedAttack | 1 << 32
        SkillTypeSpearAttack         = SkillTypeRangedAttack | 2 << 32
    SkillTypeMeleeAttack         = SkillTypeAttack | 2 << 16
        SkillTypeAxeAttack           = SkillTypeMeleeAttack | 1 << 32
        SkillTypeDaggerAttack        = SkillTypeMeleeAttack | 2 << 32
            SkillTypeLeadAttack          = SkillTypeDaggerAttack | 1 << 48
            SkillTypeOffHandAttack       = SkillTypeDaggerAttack | 2 << 48
            SkillTypeDualAttack          = SkillTypeDaggerAttack | 4 << 48
        SkillTypeHammerAttack        = SkillTypeMeleeAttack | 4 << 32
        SkillTypeScyteAttack         = SkillTypeMeleeAttack | 8 << 32
        SkillTypeSwordAttack         = SkillTypeMeleeAttack | 16 << 32
    SkillTypePetAttack           = SkillTypeAttack | 4 << 16
SkillTypeShout = 2
SkillTypeChant = 4
SkillTypeEcho = 8
SkillTypeForm = 16
SkillTypeGlypthe = 32
SkillTypePreparation = 64
SkillTypeRitual = 128
    SkillTypeBindingRitual       = SkillTypeRitual | 1 << 16
    SkillTypeNatureRitual        = SkillTypeRitual | 2 << 16
SkillTypeSignet = 256
SkillTypeSpell = 512
    SkillTypeEnchantment         = SkillTypeSpell | 1 << 16
        SkillTypeFlashEnchantment    = SkillTypeEnchantment | 1 << 32
    SkillTypeHex                 = SkillTypeSpell | 2 << 16
    SkillTypeItemSpell           = SkillTypeSpell | 4 << 16
    SkillTypeWardSpell           = SkillTypeSpell | 8 << 16
    SkillTypeWeaponSpell         = SkillTypeSpell | 16 << 16
    SkillTypeWellSpell           = SkillTypeSpell | 32 << 16
SkillTypeStance = 1024
SkillTypeTrap = 2048
SkillTypeAll = 0xFFFFFFFFFFFFFFFF

-- Effects skills can have
SkillEffectNone              = 0
SkillEffectResurrect         = 1 <<  1
SkillEffectHeal              = 1 <<  2
SkillEffectProtect           = 1 <<  3
SkillEffectDamage            = 1 <<  4
SkillEffecSpeed              = 1 <<  5
SkillEffectInterrupt         = 1 <<  6
SkillEffectGainEnergy        = 1 <<  7
SkillEffectRemoveEnchantment = 1 <<  8
SkillEffectRemoveHex         = 1 <<  9
SkillEffectRemoveStance      = 1 << 10
-- What the skill targets
SkillTargetNone         = 0
SkillTargetSelf         = 1 << 1
SkillTargetTarget       = 1 << 2
SkillTargetAoe          = 1 << 3
SkillTargetParty        = 1 << 4
-- Needed target for the skill
SkillTargetTypeNone = 0
SkillTargetTypeAlly = 1
SkillTargetTypeFoe = 2
-- Error codes
SkillErrorNone          = 0
SkillErrorInvalidSkill  = 1
SkillErrorInvalidTarget = 2
SkillErrorOutOfRange    = 3
SkillErrorNoEnergy      = 4
SkillErrorNoAdrenaline  = 5
SkillErrorRecharging    = 6
SkillErrorTargetUndestroyable = 7
SkillErrorCannotUseSkill = 8
SkillErrorNotAppropriate = 255

-- Effect categories
EffectCategoryNone        = 0
EffectCategoryCondition   = 1
EffectCategoryEnchantment = 2
EffectCategoryHex         = 3
EffectCategoryPreparation = 4
EffectCategoryShout       = 5
EffectCategorySpirit      = 6
EffectCategoryStance      = 7
EffectCategoryWard        = 8
EffectCategortyWell       = 9
EffectCategoryGeneral     = 20
EffectCategoryEnvironment = 254
