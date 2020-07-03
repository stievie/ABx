include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")
include("/scripts/includes/dialogs.lua")

name = "Telemachos Makos (Merchant)"
level = 20
itemIndex = 9     -- Merchant body model
sex = SEX_MALE
creatureState = CREATURESTATE_IDLE
prof1Index = 1     -- Warrior
prof2Index = 0     -- None

function onInit()
  return true
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
