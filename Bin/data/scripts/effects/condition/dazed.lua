include("/scripts/includes/skill_consts.lua")

function getSkillCost(skill, activation, energy, adrenaline, overcast, hp)
  return math.floor(activation * 2), energy, adrenaline, overcast, hp
end

function onGettingAttacked(source, target)
  source:InterruptSkill(SkillTypeSkill)
end
