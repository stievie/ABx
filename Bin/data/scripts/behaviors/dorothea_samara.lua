include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(interruptSpell())
    prio:AddNode(checkEnergy())
    prio:AddNode(wander())
    prio:AddNode(idle(1000))

  root:AddNode(prio)
end
