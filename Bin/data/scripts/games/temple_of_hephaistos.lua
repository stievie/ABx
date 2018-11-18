-- Game start up
function onStart()
  local portal = self:AddNpc("/scripts/actors/logic/portal.lua")
  if (portal ~= nil) then
    portal:SetName("Themple of Athene")
    -- Map ID where this portal leads to
    portal:SetVarString("destination", "75e3dfcf-479a-11e8-ad09-02100700d6f0")
    -- Will call onTrigger() when it collides
    portal:SetTrigger(true)
    local x = -20.059
    local z = -0.00870347
    local y = 26.7
    portal:SetPosition(x, y, z)
  end

  local smith = self:AddNpc("/scripts/actors/npcs/smith.lua")
  if (smith ~= nil) then
    local x = -6.71275
    local z = 15.5906
    local y = self:GetTerrainHeight(x, z)
    smith:SetPosition(x, y, z)
    smith:SetRotation(180)
    smith:SetHomePos(x, y, z)
  end  
  local merchant = self:AddNpc("/scripts/actors/npcs/merchant.lua")
  if (merchant ~= nil) then
    local x = 4.92965
    local z = 14.2049
    local y = self:GetTerrainHeight(x, z)
    merchant:SetPosition(x, y, z)
    merchant:SetRotation(180)
    merchant:SetHomePos(x, y, z)
    merchant:SetBehaviour("patrol")
  end  
  local ped = self:AddNpc("/scripts/actors/npcs/pedestrian.lua")
  if (ped ~= nil) then
    local x = 64.6874
    local z = 22.0684
    local y = self:GetTerrainHeight(x, z)
    ped:SetPosition(x, y, z)
    ped:SetRotation(90)
    ped:SetHomePos(x, y, z)
  end  
  local ped2 = self:AddNpc("/scripts/actors/npcs/pedestrian2.lua")
  if (ped2 ~= nil) then
    local x = 4.92965
    local z = 11.2049
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition(x, y, z)
    ped2:SetHomePos(x, y, z)
  end  
end

-- Game stopping
function onStop()
end

function onAddObject(object)
--  print("Object added: " .. object:GetName())
end

function onRemoveObject(object)
--  print("Object removed: " .. object:GetName())
end

function onPlayerJoin(player)
  player:AddEffect(empty, 1000, 0)
--  print("Player joined: " .. player:GetName())
end

function onPlayerLeave(player)
  player:RemoveEffect(1000)
--  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(timeElapsed)
--  print(timeElapsed)
end
