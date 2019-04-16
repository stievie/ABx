-- https://github.com/mgerhardy/engine/blob/master/src/server/lua/ai

require("data/scripts/ai/shared")
require("data/scripts/ai/smith")
require("data/scripts/ai/priest")
require("data/scripts/ai/guild_lord")

function init()
  initSmith()
  initPriest()
  initGuildLord()
end
