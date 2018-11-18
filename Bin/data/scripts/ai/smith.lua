require("data/scripts/ai/shared")

function initSmith()
  local name = "SMITH"
	local rootNode = AI.createTree(name):createRoot("PrioritySelector", name)
  idle(rootNode)
end
