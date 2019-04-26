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

function getDamage(_type, value, critical)
  return value, critical
end

function getAttackSpeed(weapon, value)
  return value
end

function getAttackDamageType(_type)
  return _type
end

function getAttackDamage(value)
  return value
end

function getArmor(damageType, value)
  return value
end

function getArmorPenetration(value)
  return value
end

function getAttributeValue(index, value)
  return value
end

function onAttack(source, target)
  return true
end

function onAttacked(source, target, _type, damage)
  return true
end

function onGettingAttacked(source, target)
  return true
end

function onUseSkill(source, target, skill)
  return true
end

function onSkillTargeted(source, target, skill)
  return true
end

function onInterruptingAttack()
  return true
end

function onInterruptingSkill(skillType, skill)
  -- Skill is going to be interrupted. If this effect prevents interrupting return false.
  return true
end

function onKnockingDown(source, target, time)
  return true
end

function onHealing(source, target, value)
  return value
end

function onGetCriticalHit(source, target)
  -- Return false when this effect prevents critical hits
  return true
end

function getResources(maxHealth, maxEnergy)
  return maxHealth, maxEnergy
end
