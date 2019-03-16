include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

-- Morale is forever but can change
isPersistent = false
internal = false

function getDuration(source, target)
  return TIME_FOREVER
end

function onStart(source, target)
  return true
end

function onEnd(source, target)
end

function onUpdate(timeElapsed)
end

function onRemove(source, target)
end

function getSkillCost(skill, activation, energy, adrenaline, overcast, hp)
  return activation, energy, adrenaline, overcast, hp
end

function getDamage(_type, value)
  return 0
end

function getAttackSpeed(weapon, value)
  return 0
end

function getAttackDamageType(_type)
  return _type
end

function getAttackDamage(value)
  return 0
end

function onAttack(source, target)
  -- We can attack others
  return true
end

function onAttacked(source, target, _type, damage)
  -- We can not be attacked
  return false
end

function onGettingAttacked(source, target)
  -- We can not getting attacked
  return false
end

function onUseSkill(source, target, skill)
  -- We could use a skill
  return true
end

function onSkillTargeted(source, target, skill)
  -- We can not be the target of a skill
  return false
end