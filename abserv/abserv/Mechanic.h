#pragma once

#include <stdint.h>
#include <limits>

// Game mechanic constants

namespace Game {

namespace {
template<typename T>
constexpr T GetPercent(T max, T percent)
{
    return (max / static_cast<T>(100)) * percent;
}
}

// Max level a player can reach
constexpr uint32_t LEVEL_CAP = 20;
constexpr uint32_t SKILLPOINT_ADVANCE_XP = 15000;
// Attributes
constexpr uint32_t MAX_ATTRIBUTES = 200;
constexpr uint32_t ADVANCE_ATTRIB_2_10 = 5;
constexpr uint32_t ADVANCE_ATTRIB_11_15 = 10;
constexpr uint32_t ADVANCE_ATTRIB_16_20 = 15;

// Resources
constexpr int BASE_ENERGY = 20;
constexpr int MAX_HEALTH_REGEN = 10;
constexpr float MAX_ENERGY_REGEN = 10.0f;

// Attack speeds in ms
constexpr uint32_t ATTACK_SPEED_AXE        = 1330;
constexpr uint32_t ATTACK_SPEED_SWORD      = 1330;
constexpr uint32_t ATTACK_SPEED_HAMMER     = 1750;
constexpr uint32_t ATTACK_SPEED_FLATBOW    = 2000;
constexpr uint32_t ATTACK_SPEED_HORNBOW    = 2700;
constexpr uint32_t ATTACK_SPEED_SHORTBOW   = 2000;
constexpr uint32_t ATTACK_SPEED_LONGBOW    = 2400;
constexpr uint32_t ATTACK_SPEED_RECURVEBOW = 2400;
constexpr uint32_t ATTACK_SPEED_STAFF      = 1750;
constexpr uint32_t ATTACK_SPEED_WAND       = 1750;
constexpr uint32_t ATTACK_SPEED_DAGGERS    = 1330;
constexpr uint32_t ATTACK_SPEED_SCYTE      = 1500;
constexpr uint32_t ATTACK_SPEED_SPEAR      = 1500;

constexpr float MAX_IAS = 1.33f;                    // Increased Attack Speed
constexpr float MAX_DAS = 0.5f;                     // Decreased Attack Speed

// For simplicity we use a sphere collision shape with this radius for projectiles
constexpr float PROJECTILE_SIZE = 0.3f;

// Ranges
constexpr float RANGE_BASE         = 80.0f;
constexpr float RANGE_AGGRO        = GetPercent(RANGE_BASE, 24.0f);
constexpr float RANGE_COMPASS      = GetPercent(RANGE_BASE, 95.0f);
constexpr float RANGE_HALF_COMPASS = RANGE_COMPASS / 2.0f;
constexpr float RANGE_SPIRIT       = RANGE_AGGRO * 1.6f;                   // Longbow, spirits
constexpr float RANGE_EARSHOT      = RANGE_AGGRO;
constexpr float RANGE_CASTING      = RANGE_AGGRO * 1.35f;
constexpr float RANGE_PROJECTILE   = RANGE_AGGRO;

constexpr float RANGE_LONGBOW      = RANGE_AGGRO * 1.6f;
constexpr float RANGE_FLATBOW      = RANGE_AGGRO * 1.6f;
constexpr float RANGE_HORNBOW      = RANGE_AGGRO * 1.35f;
constexpr float RANGE_RECURVEBOW   = RANGE_AGGRO * 1.35f;
constexpr float RANGE_SHORTBOW     = RANGE_AGGRO * 1.05f;
constexpr float RANGE_SPEAR        = RANGE_AGGRO * 1.05f;

// Close range
constexpr float RANGE_TOUCH        = 1.5f;
constexpr float RANGE_ADJECENT     = GetPercent(RANGE_BASE, 3.0f);
constexpr float RANGE_VISIBLE      = RANGE_AGGRO;

enum class Ranges : uint8_t
{
    Aggro = 0,
    Compass,
    Spirit,
    Earshot,
    Casting,
    Projectile,
    HalfCompass,
    Touch,
    Adjecent,
    Visible,
    Map                        // Whole map, must be last
};

constexpr float RangeDistances[] = {
    RANGE_AGGRO,
    RANGE_COMPASS,
    RANGE_SPIRIT,
    RANGE_EARSHOT,
    RANGE_CASTING,
    RANGE_PROJECTILE,
    RANGE_HALF_COMPASS,
    RANGE_TOUCH,
    RANGE_ADJECENT,
    RANGE_VISIBLE,
    std::numeric_limits<float>::max()
};

// https://wiki.guildwars.com/wiki/Armor_rating
constexpr float DamagePosChances[] = {
    0.125f,                                // Head
    0.375f,                                // Chest
    0.125f,                                // Hands
    0.25f,                                 // Legs
    0.125f,                                // Feet
};

// If the rotation of 2 actors is smaller than this, an attack is from behind
constexpr float BEHIND_ANGLE = 0.52f;
constexpr uint32_t DEFAULT_KNOCKDOWN_TIME = 2000;

// Item value at lvl20
constexpr unsigned int MIN_ITEM_VALUE = 10;
constexpr unsigned int MAX_ITEM_VALUE = 100;

// --- Inventory ---
constexpr uint32_t MAX_INVENTOREY_MONEY = 1000 * 1000;  // 1M
constexpr uint32_t DEFAULT_CHEST_MONEY = MAX_INVENTOREY_MONEY * 100;
constexpr uint32_t MAX_INVENTORY_STACK_SIZE = 250;
constexpr uint32_t MAX_CHEST_STACK_SIZE = 250;

// If the difference is smaller than this we will say it's the same position.
// Should be smaller than RANGE_TOUCH.
constexpr float AT_POSITON_THRESHOLD = RANGE_TOUCH - 0.1f;

}

namespace AI {

/// The AI will consider healing if HP is bellow this health/max health ratio
constexpr float LOW_HP_THRESHOLD = 0.8f;            // Percent
constexpr int CRITICAL_HP_THRESHOLD = 20;           // Absolute
constexpr float LOW_ENERGY_THRESHOLD = 1.0f;        // Percent

}
