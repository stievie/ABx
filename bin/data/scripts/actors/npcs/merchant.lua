include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/dialogs.lua")
include("/scripts/includes/items.lua")

name = "Telemachos Makos (Merchant)"
level = 20
itemIndex = 9     -- Merchant body model
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = PROFESSIONINDEX_WARRIOR
prof2Index = PROFESSIONINDEX_NONE

function onInit()
  return true
end

function getSellingItemTypes()
  return { 
    ITEMTYPE_ArmorHead,
    ITEMTYPE_ArmorChest,
    ITEMTYPE_ArmorHands,
    ITEMTYPE_ArmorLegs,
    ITEMTYPE_ArmorFeet,
    ITEMTYPE_ModifierInsignia,
    ITEMTYPE_ModifierRune,
    ITEMTYPE_ModifierWeaponPrefix,
    ITEMTYPE_ModifierWeaponSuffix,
    ITEMTYPE_ModifierWeaponInscription,
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
    ITEMTYPE_Shield,
    ITEMTYPE_Material,
    ITEMTYPE_Dye
  }
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
  player:TriggerDialog(self:GetId(), DIALOG_MERCHANT_ITEMS)
end

-- self was selected by creature
function onSelected(creature)
  local player = creature:AsPlayer()
  if (player ~= nil) then
    self:Whisper(player, "Pssst. I have some nice stuff to sell, come here and see!")
  end
end

function onAttacked(source, _type, damage, success)
  if (self:IsAttackingActor(source) == false) then
    self:Say(CHAT_CHANNEL_GENERAL, "WTF, wait, I will give you that!")
    self:Attack(source)
  end
end

-- creature collides with self
function onCollide(creature)
end
