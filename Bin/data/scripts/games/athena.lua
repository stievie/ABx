include("/scripts/includes/consts.lua")
include("/scripts/includes/create_npcs.lua")

local dorothea_samara_waypoints = {
  {  -2.4, 0.0,  -9.9 },
  { -20.0, 0.0, -32.7 },
  { -47.6, 0.0, -47.8 },
  {   6.2, 0.0, -12.2 },
  {  38.2, 0.0, -11.2 },
  {   5.8, 0.0,   9.9 },
}
local marianna_gani_waypoints = {
  {  48.5, 0.0,  19.4 },
  {  35.9, 0.0,  47.8 },
  { -25.9, 0.0,  41.2 },
  {  -6.1, 0.0,  -9.5 },
  {  40.1, 0.0, -35.4 },
  {  75.9, 0.0, -28.8 },
}

-- Game start up
function onStart()
  createPortal(self, -20.059, 26.7, -0.00870347, "Athena Arena", "3c081fd5-3966-433a-bc61-50a33084eac2")
--  createPortal(self, -20.059, 26.7, -0.00870347, "Rhodes Arena", "a13b71f8-fe19-4bf5-bba7-c7642c796c0f")

  local smith = self:AddNpc("/scripts/actors/npcs/smith.lua")
  if (smith ~= nil) then
    local x = -6.71275
    local z = 15.5906
    local y = self:GetTerrainHeight(x, z)
    smith:SetPosition({x, y, z})
    smith:SetRotation(180)
    smith:SetHomePos({x, y, z})
    smith:AddFriendFoe(GROUPMASK_1 | GROUPMASK_2, 0)
  end
  local merchant = self:AddNpc("/scripts/actors/npcs/merchant.lua")
  if (merchant ~= nil) then
    local x = 1.66
    local z = 19
    local y = self:GetTerrainHeight(x, z)
    merchant:SetPosition({x, y, z})
    merchant:SetRotation(180)
    merchant:SetHomePos({x, y, z})
    merchant:AddFriendFoe(GROUPMASK_1 | GROUPMASK_2, 0)
  end
  local ped = self:AddNpc("/scripts/actors/npcs/marianna_gani.lua")
  if (ped ~= nil) then
    local x = 64.6874
    local z = 22.0684
    local y = self:GetTerrainHeight(x, z)
    ped:SetPosition({x, y, z})
    ped:SetRotation(90)
    ped:SetHomePos({x, y, z})
    ped:AddFriendFoe(GROUPMASK_1 | GROUPMASK_2, 0)
    ped:SetWander(true)
    ped:AddWanderPoints(marianna_gani_waypoints)
  end
  local ped2 = self:AddNpc("/scripts/actors/npcs/dorothea_samara.lua")
  if (ped2 ~= nil) then
    local x = 4.08
    local z = 16.6
--    local x = 4.1
--    local z = 8.1
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition({x, y, z})
    ped2:SetHomePos({x, y, z})
    ped2:AddFriendFoe(GROUPMASK_1 | GROUPMASK_2, 0)
    ped2:SetWander(true)
    -- Add current position as first point
--    ped2:AddWanderPoint({x, y, z})
    ped2:AddWanderPoints(dorothea_samara_waypoints)
  end

  local chest = createChest(self, 0.8, 15.0)
  if (chest ~= nil) then
    chest:SetRotation(180)
  end

  -- Add some poison
  self:AddAreaOfEffect(
    "/scripts/actors/aoe/general/magic_mushroom.lua",
    nil, 10001, {-14.8, 0.0, 22.98})

  local pdl = self:AddNpc("/scripts/actors/npcs/poison_dart_launcher.lua")
  if (pdl ~= nil) then
    local x = -46
    local z = 9
    local y = self:GetTerrainHeight(x, z)
    pdl:SetPosition({x, y, z})
    -- NPCs group mask 1 -> don't shoot at NPCs
    -- Foe with 2 -> shoot at players
    pdl:AddFriendFoe(GROUPMASK_1, GROUPMASK_2)
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
  if (player ~= nil) then
    player:AddEffect(empty, 1000, 0)
    player:AddFriendFoe(GROUPMASK_2, 0)
  end
end

function onPlayerLeave(player)
  if (player ~= nil) then
    player:RemoveEffect(1000)
  end
--  print("Player left: " .. player:GetName())
end

-- Game Update
function onUpdate(timeElapsed)
--  print(timeElapsed)
end
