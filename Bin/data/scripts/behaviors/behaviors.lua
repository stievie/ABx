-- Autload these behavior trees

function init()
  -- CreateTree(name, script) creates and loads a whole BT, returns a Root node
  -- NPCs use this name to set the bevavior
  cache:Add(self:CreateTree("guild_lord", "/scripts/behaviors/guild_lord.lua"))
  cache:Add(self:CreateTree("marianna_gani", "/scripts/behaviors/marianna_gani.lua"))
  cache:Add(self:CreateTree("priest", "/scripts/behaviors/priest.lua"))
  cache:Add(self:CreateTree("smith", "/scripts/behaviors/smith.lua"))
end
