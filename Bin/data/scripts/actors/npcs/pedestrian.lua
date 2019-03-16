include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/skill_consts.lua")

name = "Pedestrian"
level = 20
modelIndex = 10    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = 3     -- Monk
prof2Index = 0     -- None
--behavior = "wander"

local rezzTarget = nil

function onInit()
  self:SetSpeed(0.5)
  self:AddEffect(empty, 900000)
  -- Let's make it a rezz machine :D
  local skillBar = self:GetSkillBar()
  -- Instant rezz skill
  skillBar:AddSkill(9996)
  return true
end

function onUpdate(timeElapsed)
  if (self:IsDead() == false and rezzTarget == nil) then
    local actors = self:GetActorsInRange(RANGE_CASTING)
    for i, v in ipairs(actors) do
      if (v:IsDead()) then
        rezzTarget = v
        break
      end
    end
    if (rezzTarget ~= nil) then
      self:FollowObject(rezzTarget)
      self:Say(CHAT_CHANNEL_GENERAL, rezzTarget:GetName() .. ", you noob!")
    end
  end

end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
  self:Say(CHAT_CHANNEL_GENERAL, "Not now!")
end

-- creature collides with self
function onCollide(creature)
end

function onArrived()
  if (rezzTarget ~= nil) then
    local skillBar = self:GetSkillBar()
    local skills = skillBar:GetSkillsWithEffect(SkillEffectResurrect)
    if (skills[1] ~= nil) then
      self:SetSelectedObject(rezzTarget)
      self:UseSkill(skills[1])
    end
    rezzTarget = nil
  end
end

function onEndUseSkill(skill)
  rezzTarget = nil
  self:Say(CHAT_CHANNEL_GENERAL, "Phew!")
end

function onStartUseSkill(skill)
end

function onDied()
  self:Say(CHAT_CHANNEL_GENERAL, "Aaaaarrrrrrggghhh")
end

function onResurrected()
  self:Say(CHAT_CHANNEL_GENERAL, "Oh, ty")
end
