include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(damageSkill())
    prio:AddNode(checkEnergy())
    prio:AddNode(defend())
    prio:AddNode(wander())
    prio:AddNode(idle(1000))

  root:AddNode(prio)
end
