include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

costEnergy = 10
costAdrenaline = 0
activation = 2
recharge = 20000
overcast = 0
effect = SkillEffectDamage
effectTarget = SkillTargetAoe
targetType = SkillTargetTypeNone

function onStartUse(source, target)
  return SkillErrorNone
end

function onSuccess(source, target)
  if (source:IsDead()) then
    return SkillErrorInvalidTarget
  end

  local game = source:GetGame()
  local pos = source:GetPosition()
  local trap = game:AddAreaOfEffect("/scripts/actors/aoe/ranger/spike_trap.lua",
    source, 461, pos)

  return SkillErrorNone
end
