include("/scripts/includes/chat.lua")
include("/scripts/includes/consts.lua")

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
  if (creature ~= nil) then
    self:FaceObject(creature)
  end
end

-- self was selected by creature
function onSelected(creature)
  if (creature == nil) then
    return;
  end

  if (creature:IsDead()) then
    self:Say(CHAT_CHANNEL_GENERAL, "Wow, how did you manage to die here? Noob!")
  else
    self:Say(CHAT_CHANNEL_GENERAL, "What do you want?!?")
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
