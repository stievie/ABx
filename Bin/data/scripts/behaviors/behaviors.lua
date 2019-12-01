-- Autoload these behavior trees into the cache

function init(cache)
  -- tree(name, script) creates and loads a whole BT, returns a Root node
  -- NPCs use this name to set the bevavior
  cache:Add(tree("guild_lord", "/scripts/behaviors/guild_lord.lua"))
  cache:Add(tree("marianna_gani", "/scripts/behaviors/marianna_gani.lua"))
  cache:Add(tree("dorothea_samara", "/scripts/behaviors/dorothea_samara.lua"))
  cache:Add(tree("priest", "/scripts/behaviors/priest.lua"))
  cache:Add(tree("smith", "/scripts/behaviors/smith.lua"))
  cache:Add(tree("elementarist", "/scripts/behaviors/elementarist.lua"))
end
