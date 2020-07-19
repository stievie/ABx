-- Function to generate items

include("/scripts/includes/attributes.lua")
include("/scripts/includes/damage.lua")

local damageTypes = {}

damageTypes[ATTRIB_FASTCAST] = DAMAGETYPE_CHAOS
damageTypes[ATTRIB_ILLUSION] = DAMAGETYPE_CHAOS
damageTypes[ATTRIB_DOMINATION] = DAMAGETYPE_CHAOS
damageTypes[ATTRIB_INSPIRATION] = DAMAGETYPE_CHAOS
damageTypes[ATTRIB_BLOOD] = DAMAGETYPE_DARK
damageTypes[ATTRIB_DEATH] = DAMAGETYPE_DARK
damageTypes[ATTRIB_SOUL_REAPING] = DAMAGETYPE_DARK
damageTypes[ATTRIB_CURSES] = DAMAGETYPE_DARK
damageTypes[ATTRIB_AIR] = DAMAGETYPE_LIGHTNING
damageTypes[ATTRIB_EARTH] = DAMAGETYPE_EARTH
damageTypes[ATTRIB_FIRE] = DAMAGETYPE_FIRE
damageTypes[ATTRIB_WATER] = DAMAGETYPE_COLD
damageTypes[ATTRIB_ENERGY_STORAGE] = DAMAGETYPE_FIRE
damageTypes[ATTRIB_HEALING] = DAMAGETYPE_HOLY
damageTypes[ATTRIB_SMITING] = DAMAGETYPE_HOLY
damageTypes[ATTRIB_PROTECTION] = DAMAGETYPE_HOLY
damageTypes[ATTRIB_DEVINE_FAVOUR] = DAMAGETYPE_HOLY
damageTypes[ATTRIB_AXE_MASTERY] = DAMAGETYPE_SLASHING
damageTypes[ATTRIB_HAMMER_MASTERY] = DAMAGETYPE_BLUNT
damageTypes[ATTRIB_SWORDS_MANSHIP] = DAMAGETYPE_SLASHING
damageTypes[ATTRIB_MARK_MANSSHIP] = DAMAGETYPE_PIERCING

function getRandomStat(maxVal, level, minVal)
  if (level == 20 and Random() >= 0.3) then
    -- For level 20 the chance to get max stats is 70%
    return maxVal
  end
  local p = level / 20
  local val = maxVal * p
  local res = val - (2 * Random())
  if (minVal ~= nil and res < minVal) then
    res = minVal
  end
  return math.floor(res)
end

-- Range is damage / 2 .. damage
function getDamageStats(level, maxStats)
  if (maxStats) then
    return dropStats["MinDamage"], dropStats["MaxDamage"]
  end

  local maxRes = getRandomStat(dropStats["MaxDamage"], level, 3)
  if (maxRes == dropStats["MaxDamage"]) then
    -- Max -> return max min damage
    return dropStats["MinDamage"], maxRes
  end

  local diff = dropStats["MaxDamage"] - dropStats["MinDamage"]
  local p = level / 20
  local  minRes = math.floor(maxRes - (diff * p))
  if (minRes < 1) then
    minRes = 1
  end
  return minRes, maxRes
end

function getDamageTypeStats(level, maxStats, attribute)
  if (damageTypes[attribute] == nil) then
    return DAMAGETYPE_UNKNOWN
  end
  return damageTypes[attribute]
end

function getEnergyStats(level, maxStats)
  if (maxStats) then
    return dropStats["Energy"]
  end
  return getRandomStat(dropStats["Energy"], level, 1)
end

function getArmorStats(level, maxStats)
  if (maxStats) then
    return dropStats["Armor"]
  end
  return getRandomStat(dropStats["Armor"], level, 6)
end

function getHealthStats(level, maxStats)
  if (maxStats) then
    return dropStats["Health"]
  end
  return getRandomStat(dropStats["Health"], level)
end

function getUsagesStats(level, maxStats)
  if (maxStats) then
    return dropStats["Usages"]
  end
  return getRandomStat(dropStats["Usages"], level)
end

function getAtttributes()
  if (dropStats["Attributes"] == nil) then
    print("ERROR: dropStats[Attributes] == nil")
    return { ATTRIB_NONE }
  end
  return dropStats["Attributes"]
end

function getRandomAttribute()
  if (dropStats["Attributes"] == nil) then
    print("ERROR: dropStats[Attributes] == nil")
    return ATTRIB_NONE
  end
  local rnd = math.floor(Random(1, #dropStats["Attributes"]))

  return dropStats["Attributes"][rnd]
end

function getRequirement(level, maxStat)
  local tbl = {
    0,             -- 1
    0,             -- 2
    1,             -- 3
    1,             -- 4
    2,             -- 5
    2,             -- 6
    3,             -- 7
    4,             -- 8
    5,             -- 9
    5,             -- 10
    6,             -- 11
    6,             -- 12
    7,             -- 13
    7,             -- 14
    8,             -- 15
    8,             -- 16
    9,             -- 17
    9,             -- 18
    9,             -- 19
    9              -- 20
  }

  if (maxStat) then
    return tbl[level]
  end
  local rnd = math.floor(Random(0, 4))
  local val = tbl[level] + rnd;
  if (val > 13) then
    val = 13
  end
  return val
end

function getAttributeStats(level, maxStat)
  return getRandomAttribute(), getRequirement(level, maxStat)
end

function getValueStat(index, level, maxStat)
  if (materialStats == nil) then
    return 0, 0
  end
  if (materialStats[index] == nil) then
    print("ERROR: materialStats[" .. index .. "] == nil", materialStats)
    return 0, 0
  end
  local itemIndex = materialStats[index][1]
  local itemCount = materialStats[index][2]
  if (maxStat) then
    return itemIndex, itemCount
  end
  return itemIndex, getRandomStat(itemCount, level)
end
