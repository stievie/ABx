include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

-- Morale is forever but can change
isPersistent = false

function getDuration(source, target)
  return TIME_FOREVER
end

function onStart(source, target)
  local skillBar = target:GetSkillBar()
  for i = 1, MAX_SKILLS, 1 do
    local skill = skillBar:GetSkill(i - 1)
    if (skill ~= nil) then
      skill:SetRecharged(0)
    end
  end
  return true
end

function onEnd(source, target)
end

function onRemove(source, target)
end
