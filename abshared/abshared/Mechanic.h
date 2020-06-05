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

#include <stdint.h>
#include <limits>

// Game mechanic constants. This is here because server and client may share some
// of the values here.

namespace Game {

template<typename T>
constexpr T GetPercent(T max, T percent)
{
    return (max / static_cast<T>(100)) * percent;
}

inline constexpr int PLAYER_MAX_SKILLS = 8;
// Most profession have 4 attribute but Warrior and Elementarist have 5
inline constexpr int PLAYER_MAX_ATTRIBUTES = 10;

// For client prediction these values are also needed by the client.
inline constexpr float BASE_MOVE_SPEED = 150.0f;
inline constexpr float BASE_TURN_SPEED = 2000.0f;

// Max level a player can reach
inline constexpr uint32_t LEVEL_CAP = 20;
inline constexpr uint32_t SKILLPOINT_ADVANCE_XP = 15000;
// Attributes
inline constexpr uint32_t MAX_ATTRIBUTES = 200;
inline constexpr uint32_t ADVANCE_ATTRIB_2_10 = 5;
inline constexpr uint32_t ADVANCE_ATTRIB_11_15 = 10;
inline constexpr uint32_t ADVANCE_ATTRIB_16_20 = 15;
inline constexpr uint32_t MAX_PLAYER_ATTRIBUTE_RANK = 12;

// Resources
inline constexpr int BASE_ENERGY = 20;
inline constexpr int MAX_HEALTH_REGEN = 10;
inline constexpr float MAX_ENERGY_REGEN = 10.0f;

// Attack speeds in ms
inline constexpr uint32_t ATTACK_SPEED_AXE        = 1330;
inline constexpr uint32_t ATTACK_SPEED_SWORD      = 1330;
inline constexpr uint32_t ATTACK_SPEED_HAMMER     = 1750;
inline constexpr uint32_t ATTACK_SPEED_FLATBOW    = 2000;
inline constexpr uint32_t ATTACK_SPEED_HORNBOW    = 2700;
inline constexpr uint32_t ATTACK_SPEED_SHORTBOW   = 2000;
inline constexpr uint32_t ATTACK_SPEED_LONGBOW    = 2400;
inline constexpr uint32_t ATTACK_SPEED_RECURVEBOW = 2400;
inline constexpr uint32_t ATTACK_SPEED_STAFF      = 1750;
inline constexpr uint32_t ATTACK_SPEED_WAND       = 1750;
inline constexpr uint32_t ATTACK_SPEED_DAGGERS    = 1330;
inline constexpr uint32_t ATTACK_SPEED_SCYTE      = 1500;
inline constexpr uint32_t ATTACK_SPEED_SPEAR      = 1500;

inline constexpr float MAX_IAS = 1.33f;                    // Increased Attack Speed
inline constexpr float MAX_DAS = 0.5f;                     // Decreased Attack Speed
inline constexpr int MAX_MORALE = 10;
inline constexpr int MIN_MORALE = -60;

// For simplicity we use a sphere collision shape with this radius for projectiles
inline constexpr float PROJECTILE_SIZE = 0.3f;

// Ranges
inline constexpr float RANGE_BASE         = 80.0f;
inline constexpr float RANGE_AGGRO        = GetPercent(RANGE_BASE, 24.0f);
inline constexpr float RANGE_COMPASS      = GetPercent(RANGE_BASE, 95.0f);
inline constexpr float RANGE_HALF_COMPASS = RANGE_COMPASS / 2.0f;
inline constexpr float RANGE_TWO_COMPASS  = RANGE_COMPASS * 2.0f;
inline constexpr float RANGE_SPIRIT       = RANGE_AGGRO * 1.6f;                   // Longbow, spirits
inline constexpr float RANGE_EARSHOT      = RANGE_AGGRO;
inline constexpr float RANGE_CASTING      = RANGE_AGGRO * 1.35f;
inline constexpr float RANGE_PROJECTILE   = RANGE_AGGRO;
inline constexpr float RANGE_INTEREST     = RANGE_COMPASS * 3.0f;                 // What happens inside this range is sent to the player

inline constexpr float RANGE_LONGBOW      = RANGE_AGGRO * 1.6f;
inline constexpr float RANGE_FLATBOW      = RANGE_AGGRO * 1.6f;
inline constexpr float RANGE_HORNBOW      = RANGE_AGGRO * 1.35f;
inline constexpr float RANGE_RECURVEBOW   = RANGE_AGGRO * 1.35f;
inline constexpr float RANGE_SHORTBOW     = RANGE_AGGRO * 1.05f;
inline constexpr float RANGE_SPEAR        = RANGE_AGGRO * 1.05f;

// Close range
inline constexpr float RANGE_TOUCH        = 1.5f;
inline constexpr float RANGE_ADJECENT     = GetPercent(RANGE_BASE, 3.0f);
inline constexpr float RANGE_VISIBLE      = RANGE_AGGRO;
inline constexpr float RANGE_SELECT       = RANGE_COMPASS + 2.0f;
inline constexpr float RANGE_PICK_UP      = 2.5f;

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
    TwoCompass,
    Interest,
    Map                        // Whole map, must be last
};

inline constexpr float RangeDistances[] = {
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
    RANGE_TWO_COMPASS,
    RANGE_INTEREST,
    std::numeric_limits<float>::max()
};

// https://wiki.guildwars.com/wiki/Armor_rating
inline constexpr float DamagePosChances[] = {
    0.125f,                                // Head
    0.375f,                                // Chest
    0.125f,                                // Hands
    0.25f,                                 // Legs
    0.125f,                                // Feet
};

// If the rotation of 2 actors is smaller than this, an attack is from behind
inline constexpr float BEHIND_ANGLE = 0.52f;
inline constexpr uint32_t DEFAULT_KNOCKDOWN_TIME = 2000;

// Item value at lvl20
inline constexpr unsigned int MIN_ITEM_VALUE = 10;
inline constexpr unsigned int MAX_ITEM_VALUE = 100;

// --- Inventory ---
inline constexpr uint32_t MAX_INVENTOREY_MONEY = 1000 * 1000;  // 1M
inline constexpr uint32_t DEFAULT_CHEST_MONEY = MAX_INVENTOREY_MONEY * 100; // 100M
inline constexpr uint32_t MAX_INVENTORY_STACK_SIZE = 250;
inline constexpr uint32_t MAX_CHEST_STACK_SIZE = 250;

// If the difference is smaller than this we will say it's the same position.
// Should be smaller than RANGE_TOUCH.
inline constexpr float AT_POSITION_THRESHOLD = RANGE_TOUCH - 0.1f;

}

namespace AI {

/// The AI will consider healing if HP is bellow this health/max health ratio
inline constexpr float LOW_HP_THRESHOLD = 0.8f;            // Percent
inline constexpr int CRITICAL_HP_THRESHOLD = 20;           // Absolute
inline constexpr float LOW_ENERGY_THRESHOLD = 1.0f;        // Percent

}
