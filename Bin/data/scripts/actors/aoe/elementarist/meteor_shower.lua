include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")
include("/scripts/includes/attributes.lua")

effect = SkillEffectDamage
effectTarget = SkillTargetAoe

local damage
local lastDamage

function onInit()
  local source = self:GetSource()
  if (source == nil)
    return false
  end
  
  local attribVal = source:GetAttributeValue(ATTRIB_FIRE)
  damage = 7 * attribVal
  lastDamage = Tick()
  self:SetRange(RANGE_ADJECENT)
  self:SetLifetime(9000)
  return true
end

function onUpdate(timeElapsed)
  local tick = Tick()
  if (tick - lastDamage > 3000)
    local actors = self:GetActorsInRange(self:GetRange())
    local source = self:GetSource()
    for i, actor in ipairs(actors) do
      if (actor:IsEnemy(source))
        actor:Damage(source, self:Index(), DAMAGETYPE_FIRE, damage)
        actor:KnockDown(source, 500)
      end
    end
    lastDamage = tick
  end
end

function onEnded()
end
