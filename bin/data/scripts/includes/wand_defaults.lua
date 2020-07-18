include("/scripts/includes/attributes.lua")
-- Drop Stats ------------------------------------------------------------------
dropStats = {}
materialStats = {}
-- When this item drops it has a damage in this range scaled to the level of 
-- the player/drop area
dropStats["MinDamage"] = 11
dropStats["MaxDamage"] = 22
dropStats["Attributes"] = { 
  ATTRIB_FASTCAST, 
  ATTRIB_ILLUSION,
  ATTRIB_DOMINATION,
  ATTRIB_INSPIRATION,
  ATTRIB_BLOOD,
  ATTRIB_DEATH,
  ATTRIB_SOUL_REAPING,
  ATTRIB_CURSES,
  ATTRIB_AIR,
  ATTRIB_EARTH,
  ATTRIB_FIRE,
  ATTRIB_WATER,
  ATTRIB_ENERGY_STORAGE,
  ATTRIB_HEALING,
  ATTRIB_SMITING,
  ATTRIB_PROTECTION,
  ATTRIB_DEVINE_FAVOUR
}

materialStats[1] = { 100001, 50 }
materialStats[2] = { 100000, 10 }
materialStats[3] = { 100002, 20 }
materialStats[4] = { 9999999, 750 }
-- /Drop Stats -----------------------------------------------------------------
