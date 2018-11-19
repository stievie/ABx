require("data/scripts/ai/shared")

-- Root
--  - PrioritySelector
--      - Idle
function initSmith()
  local name = "SMITH"
	local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
  idle(rootNode)
end
