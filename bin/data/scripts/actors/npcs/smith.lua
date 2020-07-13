include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/dialogs.lua")
include("/scripts/includes/attributes.lua")
include("/scripts/includes/items.lua")

name = "Dimitris Sallas (Smith)"
level = 20
itemIndex = 5     -- Smith body model
sex = SEX_MALE     -- Male
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_WARRIOR
prof2Index = PROFESSIONINDEX_NONE
behavior = "smith"

function onInit()
  local skillBar = self:GetSkillBar()
  -- Add a heal skill
  skillBar:AddSkill(1)
  skillBar:SetAttributeRank(ATTRIB_TACTICS, 12)
  skillBar:SetAttributeRank(ATTRIB_STRENGTH, 12)
  return true
end

function getSellingItemTypes()
  return { 
    ITEMTYPE_ArmorHead,
    ITEMTYPE_ArmorChest,
    ITEMTYPE_ArmorHands,
    ITEMTYPE_ArmorLegs,
    ITEMTYPE_ArmorFeet,
    ITEMTYPE_Axe,
    ITEMTYPE_Sword,
    ITEMTYPE_Hammer,
    ITEMTYPE_Flatbow,
    ITEMTYPE_Hornbow,
    ITEMTYPE_Shortbow,
    ITEMTYPE_Longbow,
    ITEMTYPE_Recurvebow,
    ITEMTYPE_Staff,
    ITEMTYPE_Wand,
    ITEMTYPE_Daggers,
    ITEMTYPE_Scyte,
    ITEMTYPE_Spear,
    ITEMTYPE_Focus,
    ITEMTYPE_Shield
  }
end

function isSellingItem(itemIndex)
  return (itemIndex == 500) or (itemIndex == 501) or (itemIndex == 502) or
    (itemIndex == 506) or (itemIndex == 507) or (itemIndex == 509) or (itemIndex == 511)
end

function onUpdate(timeElapsed)

end

function onClicked(creature)
  if (creature == nil) then
    return
  end
  self:FaceObject(creature)
  if (not self:IsInRange(RANGE_ADJECENT, creature)) then
    return
  end
  
  local player = creature:AsPlayer()
  if (player == nil) then
    return
  end
  player:TriggerDialog(self:GetId(), DIALOG_CRAFTSMAN_ITEMS)
end

-- self was selected by creature
function onSelected(creature)
end

-- creature collides with self
function onCollide(creature)
end
