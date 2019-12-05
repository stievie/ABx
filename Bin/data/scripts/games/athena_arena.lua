include("/scripts/includes/consts.lua")

local point_blue = {
  -20.0, 0.0, -27.0
}

local point_red = {
  -19.0, 0.0, -44.0
}

local function createTeam(spawn, frnd, foe, rot, color)
  -- To make them allies set the same group ID. Adding NPCs to a Crowd sets the group ID.
  local crowd = self:AddCrowd()
  crowd:SetColor(color)
  local guildLord = self:AddNpc("/scripts/actors/npcs/guild_lord.lua")
  if (guildLord ~= nil) then
    crowd:Add(guildLord)
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    guildLord:SetPosition({x, y, z})
    guildLord:SetRotation(rot)
--    guildLord:SetHomePos({x, y, z})
    guildLord:AddFriendFoe(frnd, foe)
  end

  local ped2 = self:AddNpc("/scripts/actors/npcs/dorothea_samara.lua")
  if (ped2 ~= nil) then
    crowd:Add(ped2)
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    ped2:SetPosition({x, y, z})
    ped2:SetRotation(rot)
--    ped2:SetHomePos({x, y, z})
    ped2:AddFriendFoe(frnd, foe)
  end
  local ped3 = self:AddNpc("/scripts/actors/npcs/electra_staneli.lua")
  if (ped3 ~= nil) then
    crowd:Add(ped3)
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    ped3:SetPosition({x, y, z})
    ped3:SetRotation(rot)
--    ped2:SetHomePos({x, y, z})
    ped3:AddFriendFoe(frnd, foe)
  end

  local priest = self:AddNpc("/scripts/actors/npcs/priest.lua")
  if (priest ~= nil) then
    crowd:Add(priest)
    local x = spawn[1] + Random(-1, 1)
    local z = spawn[3] + Random(-1, 1)
    local y = self:GetTerrainHeight(x, z)
    priest:SetPosition({x, y, z})
    priest:SetRotation(rot)
--    priest:SetHomePos({x, y, z})
    priest:AddFriendFoe(frnd, foe)
  end
end

function onStart()
  createTeam(point_blue, GROUPMASK_1, GROUPMASK_2 | GROUPMASK_3, -90, TEAMCOLOR_BLUE)
  createTeam(point_red, GROUPMASK_2, GROUPMASK_1 | GROUPMASK_3, -90, TEAMCOLOR_RED)
end

function onStop()
end

function onAddObject(object)
end

function onRemoveObject(object)
end

function onPlayerJoin(player)
  player:AddEffect(empty, 1000, 0)

  player:AddFriendFoe(GROUPMASK_3, GROUPMASK_1 | GROUPMASK_2)
end

function onPlayerLeave(player)
end

-- Game Update
function onUpdate(timeElapsed)
end
