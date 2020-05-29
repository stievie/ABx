include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/attributes.lua")

name = "Dorothea Samara"
level = 20
itemIndex = 12    -- Female Pedestrian 1 body model
sex = SEX_FEMALE
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_MESMER
prof2Index = PROFESSIONINDEX_NONE
behavior = "dorothea_samara"

local startTick;

function onInit()
  startTick = Tick()
  self:SetSpeed(0.5)

  local skillBar = self:GetSkillBar()
--  skillBar:AddSkill(39)
  skillBar:AddSkill(42)
  skillBar:AddSkill(61)
  skillBar:AddSkill(40)
  skillBar:SetAttributeRank(ATTRIB_FASTCAST, 10)
  skillBar:SetAttributeRank(ATTRIB_INSPIRATION, 8)
  skillBar:SetAttributeRank(ATTRIB_DOMINATION, 12)

  skillBar:AddSkill(2)

  return true
end

function onUpdate(timeElapsed)
  if (Tick() - startTick > 10000 and self:GetState() == CREATURESTATE_IDLE) then
    startTick = Tick()
  end
end

function onArrived()
end

function onClicked(creature)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end

function onDied()
  if (self:GetDeaths() < 2) then
    self:DropRandomItem()
  end
end

function onResurrected()
end
