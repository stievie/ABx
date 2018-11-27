--------------------------------------------------------------------------------
-- Game mechanics Settings -----------------------------------------------------
--------------------------------------------------------------------------------

require("config/functions")

behaviours = "/scripts/ai/behaviours.lua"

level_cap = 20

-- Ranges ----------------------------------------------------------------------
range_base = 80   -- 100% Slightly larger than the compass

range_aggro = GetValue(range_base, 24.0)

-- Long Range
range_compass = GetValue(range_base, 95.0)
range_spirit = range_aggro * 1.6  -- Longbow, spirits
-- Mid Range
range_earshot = range_aggro
range_casting = range_aggro * 1.35
range_projectile = range_aggro
range_half_compass = range_compass / 2.0
-- Close Range
range_touch = 1.5
range_adjacent = GetValue(range_base, 3.0)

range_visible = range_aggro

-- print("range_aggro " .. range_aggro)
-- print("range_compass " .. range_compass)
-- print("range_adjacent " .. range_adjacent)
-- print("range_spirit " .. range_spirit)
-- print("range_casting " .. range_casting)
