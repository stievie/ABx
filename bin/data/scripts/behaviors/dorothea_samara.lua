include("/scripts/behaviors/shared.lua")

function init(root)
  local prio = node("Priority")
    prio:AddNode(stayAlive())
    prio:AddNode(rezzAlly())
    prio:AddNode(avoidSelfDamage())
    prio:AddNode(damageSkill())
    prio:AddNode(checkEnergy())
    prio:AddNode(defend())
    prio:AddNode(wander())

  root:AddNode(prio)
end
