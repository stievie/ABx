include("/scripts/includes/attributes.lua")
-- Drop Stats ------------------------------------------------------------------
dropStats = {}
materialStats = {}
-- When this item drops it has a damage in this range scaled to the level of 
-- the player/drop area
dropStats["MinDamage"] = 15
dropStats["MaxDamage"] = 22
dropStats["Attributes"] = { ATTRIB_HAMMER_MASTERY }

materialStats[1] = { 100000, 1000 }
materialStats[2] = { 100001, 50 }
materialStats[3] = { 0, 0 }
materialStats[4] = { 9999999, 1000 }
-- /Drop Stats -----------------------------------------------------------------
