include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")

-- https://wiki.guildwars.com/wiki/Spike_Trap

itemIndex = 6000
creatureState = CREATURESTATE_IDLE
effect = SkillEffectDamage
effectTarget = SkillTargetAoe

-- 90 seconds
local lifeTime = 90 * 1000

local damage = 0;
local crippledTime = 0;

function onInit()
  local source = self:GetSource()
  if (source == nil) then
    return false
  end
  
  self:SetRange(RANGE_ADJECENT)
  self:SetLifetime(lifeTime)
  self:SetTrigger(true)
  local attribVal = source:GetAttributeRank(ATTRIB_WILDERNESS_SURVIVAL)
  damage = 10 + (attribVal * 2)
  crippledTime = 3 + math.floor(attribVal * 1.5)
  return true
end

function onTrigger(other)
  if (self:GetState() == CREATURESTATE_IDLE) then
    -- Not triggered yet, so let's trigger now
    self:SetState(CREATURESTATE_TRIGGERED)
    local actors = self:GetActorsInRange(self:GetRange())
    for i, actor in ipairs(actors) do
      actor:Damage(self:GetSource(), self:Index(), DAMAGETYPE_PIERCING, damage)
      actor:KnockDown(self:GetSource(), 0)
      actor:AddEffect(self:GetSource(), 10001, crippledTime)
    end
    self:RemoveIn(2000)
  end
end
