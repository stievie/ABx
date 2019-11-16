#pragma once

#include <map>
#include <memory>
#include <AB/Entities/Effect.h>
#include <sa/StringHash.h>

namespace Game {

class Effect;

constexpr size_t EFFECTTCAT_CONDITION = sa::StringHash("Condition");
constexpr size_t EFFECTTCAT_ENCHANTMENT = sa::StringHash("Enchantment");
constexpr size_t EFFECTTCAT_HEX = sa::StringHash("Hex");
constexpr size_t EFFECTTCAT_SHOUT = sa::StringHash("Shout");
constexpr size_t EFFECTTCAT_SPIRIT = sa::StringHash("Spirit");
constexpr size_t EFFECTTCAT_WARD = sa::StringHash("Ward");
constexpr size_t EFFECTTCAT_WELL = sa::StringHash("Well");
constexpr size_t EFFECTTCAT_PREPARATION = sa::StringHash("Preparation");
constexpr size_t EFFECTTCAT_STANCE = sa::StringHash("Stance");
constexpr size_t EFFECTTCAT_FORM = sa::StringHash("Form");
constexpr size_t EFFECTTCAT_GLYPHE = sa::StringHash("Glyphe");
constexpr size_t EFFECTTCAT_PETATTTACK = sa::StringHash("PetAttack");
constexpr size_t EFFECTTCAT_WEAPONSPELL = sa::StringHash("WeaponSpell");

AB::Entities::EffectCategory EffectCatNameToEffectCat(const std::string& name);

class EffectManager
{
private:
    std::map<uint32_t, AB::Entities::Effect> effects_;
public:
    EffectManager() = default;
    ~EffectManager() = default;

    std::shared_ptr<Effect> Get(uint32_t index);
};

}
