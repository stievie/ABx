include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")
include("/scripts/includes/damage.lua")

itemIndex = 6000
effect = SkillEffectDamage
effectTarget = SkillTargetAoe

-- https://wiki.guildwars.com/wiki/Poison_Spout
local damage = 50
local poisoningMS = 12 * 1000
local activeTime = 2000
local inactiveTime = 8000
local lastDamage
local lastActive;

function onInit()
  self:SetRange(RANGE_ADJECENT)
  self:SetLifetime(TIME_FOREVER)
  self:SetTrigger(true)
  lastDamage = Tick()
  lastActive = Tick()
  return true
end

function onUpdate(timeElapsed)
  local tick = Tick()
  if (self:GetState() == CREATURESTATE_IDLE) then
    if (tick - lastActive > inactiveTime) then
      self:SetState(CREATURESTATE_TRIGGERED)
      lastActive = tick
    end
  else
    if (tick - lastActive > activeTime + inactiveTime) then
      self:SetState(CREATURESTATE_IDLE)
      lastActive = tick
    end
  end
  
  if (self:GetState() ~= CREATURESTATE_TRIGGERED) then
    return
  end
  
  if (tick - lastDamage > 1000) then
    local objects = self:GetObjectsInside()
    for i, object in ipairs(objects) do
      local actor = object:AsActor()
      if (actor ~= nil) then
        actor:Damage(nil, self:Index(), DAMAGETYPE_COLD, damage)
        actor:AddEffect(nil, 10002, 12000)
      end
    end
    lastDamage = tick
  end
end
