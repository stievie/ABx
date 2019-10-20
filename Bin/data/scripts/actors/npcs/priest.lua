include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/skill_consts.lua")

name = "Priest"
level = 20
itemIndex = 2
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_MONK
prof2Index = PROFESSIONINDEX_NONE
behavior = "PRIEST"

function onInit()
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(281)
  skillBar:AddSkill(280)
  -- Instant rezz skill
  skillBar:AddSkill(9996)
  skillBar:SetAttributeValue(ATTRIB_HEALING, 12)
  skillBar:SetAttributeValue(ATTRIB_DEVINE_FAVOUR, 12)
  return true
end

function onUpdate(timeElapsed)
end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end

function onAttacked(source, _type, damage, success)
  if (self:IsAttackingActor(source) == false) then
--    self:Say(CHAT_CHANNEL_GENERAL, "Okay " .. source:GetName() .. ", take that!")
--    self:Attack(source)
  end
end

function onDied()
  self:DropRandomItem()
end
