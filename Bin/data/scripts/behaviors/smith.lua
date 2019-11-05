include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = self:CreateNode("Sequence")
  prio:AddNode(idle(1000))
  root:AddNode(prio)
end
