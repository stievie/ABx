-- Some shortucts to create NPCs

-- Create and account chest
function createChest(game, x, z)
  local chest = game:AddNpc("/scripts/actors/logic/account_chest.lua")
  if (chest ~= nil) then
    local y = game:GetTerrainHeight(x, z)
    chest:SetPosition({x, y, z})
    return chest
  end
  return nil
end

-- Create a portal to a map
function createPortal(game, x, y, z, name, destination)
  local portal = game:AddNpc("/scripts/actors/logic/portal.lua")
  if (portal ~= nil) then
    portal:SetName(name)
    -- Map ID where this portal leads to
    portal:SetVarString("destination", destination)
    portal:SetPosition({x, y, z})
    return portal
  end
  return nil
end
