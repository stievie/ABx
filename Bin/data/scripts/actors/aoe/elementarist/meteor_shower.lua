include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

local damage
local lastDamage

function onInit()
  local source = self:GetSource()
  if (source == nil)
    return false
  end
  
  local attribVal = source:GetAttributeValue(ATTRIB_FIRE)
  damage = 7 * attribVal
  lastDamage = GameTick()
  self:SetRange(RANGE_ADJECENT)
  self:SetLifetime(9000)
  return true
end

function onUpdate(timeElapsed)
  local tick = GameTick()
  if (tick - lastDamage > 3000)
    local actors = self:GetActorsInRange(self:GetRange())
    local source = self:GetSource()
    for i, actor in ipairs(actors) do
      if (actor:IsEnemy(source))
        actor:ApplyDamage(source, self:Index(), DAMAGETYPE_FIRE, damage, 0)
        actor:KnockDown(source, 500)
      end
    end
    lastDamage = tick
  end
end

function onEnded()
end
