#pragma once

#include <stdint.h>

// Game mechanic constants

namespace Game {

namespace {
template<typename T>
static constexpr T GetPercent(T max, T percent)
{
    return (max / static_cast<T>(100)) * percent;
}
}

// Base move speed
static constexpr float BASE_SPEED = 150.0f;

// Attack speeds in ms
static constexpr uint32_t ATTACK_SPEED_AXE        = 1330;
static constexpr uint32_t ATTACK_SPEED_SWORD      = 1330;
static constexpr uint32_t ATTACK_SPEED_HAMMER     = 1750;
static constexpr uint32_t ATTACK_SPEED_FLATBOW    = 2000;
static constexpr uint32_t ATTACK_SPEED_HORNBOW    = 2700;
static constexpr uint32_t ATTACK_SPEED_SHORTBOW   = 2000;
static constexpr uint32_t ATTACK_SPEED_LONGBOW    = 2400;
static constexpr uint32_t ATTACK_SPEED_RECURVEBOW = 2400;
static constexpr uint32_t ATTACK_SPEED_STAFF      = 1750;
static constexpr uint32_t ATTACK_SPEED_WAND       = 1750;
static constexpr uint32_t ATTACK_SPEED_DAGGERS    = 1330;
static constexpr uint32_t ATTACK_SPEED_SCYTE      = 1500;
static constexpr uint32_t ATTACK_SPEED_SPEAR      = 1500;

static constexpr float MAX_IAS = 1.33f;                    // Increased Attack Speed
static constexpr float MAX_DAS = 0.5f;                     // Decreased Attack Speed

// Ranges
static constexpr float RANGE_BASE         = 80.0f;
static constexpr float RANGE_AGGRO        = GetPercent(RANGE_BASE, 24.0f);
static constexpr float RANGE_COMPASS      = GetPercent(RANGE_BASE, 95.0f);
static constexpr float RANGE_HALF_COMPASS = RANGE_COMPASS / 2.0f;
static constexpr float RANGE_SPIRIT       = RANGE_AGGRO * 1.6f;                   // Longbow, spirits
static constexpr float RANGE_EARSHOT      = RANGE_AGGRO;
static constexpr float RANGE_CASTING      = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_PROJECTILE   = RANGE_AGGRO;

static constexpr float RANGE_LONGBOW      = RANGE_AGGRO * 1.6f;
static constexpr float RANGE_FLATBOW      = RANGE_AGGRO * 1.6f;
static constexpr float RANGE_HORNBOW      = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_RECURVEBOW   = RANGE_AGGRO * 1.35f;
static constexpr float RANGE_SHORTBOW     = RANGE_AGGRO * 1.05f;
static constexpr float RANGE_SPEAR        = RANGE_AGGRO * 1.05f;

// Close range
static constexpr float RANGE_TOUCH        = 1.5f;
static constexpr float RANGE_ADJECENT     = GetPercent(RANGE_BASE, 3.0f);
static constexpr float RANGE_VISIBLE      = RANGE_AGGRO;

}
