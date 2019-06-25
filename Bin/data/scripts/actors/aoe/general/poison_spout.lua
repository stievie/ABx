include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")

-- https://wiki.guildwars.com/wiki/Poison_Spout
local damage = 50
local poisoningMS = 12 * 1000
local lastDamage

function onInit()
  self:SetRange(RANGE_ADJECENT)
  self:SetLifetime(TIME_FOREVER)
  lastDamage = Tick()
  return true
end

function onUpdate(timeElapsed)
  local tick = Tick()
  if (tick - lastDamage > 1000) then
    local actors = self:GetActorsInRange(self:GetRange())
    for i, actor in ipairs(actors) do
      actor:ApplyDamage(nil, self:Index(), DAMAGETYPE_COLD, damage, 0)
    end
    lastDamage = tick
  end
end
