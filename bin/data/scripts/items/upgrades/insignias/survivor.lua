-- Drop Stats ------------------------------------------------------------------
dropStats = {}
materialStats = {}
-- When this item drops it has a damage in this range scaled to the level of 
-- the player/drop area
dropStats["Health"] = 5
-- /Drop Stats -----------------------------------------------------------------

materialStats[1] = { 0, 0 }
materialStats[2] = { 0, 0 }
materialStats[3] = { 0, 0 }
materialStats[4] = { 9999999, 30 }

include("/scripts/includes/item_functions.lua")
