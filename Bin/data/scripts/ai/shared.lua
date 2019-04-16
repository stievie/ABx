function idle(parentnode)
  -- This node tries to execute all the attached children until one succeeds. This composite only
  -- fails if all children failed, too.
  local prio = parentnode:addNode("PrioritySelector", "idle")
    prio:addNode("Idle{1000}", "idle1000")
end

function stayAlive(parentnode)
  -- Executes all the connected children in the order they were added (no matter what
  -- the TreeNodeStatus of the previous child was).
  local parallel = parentnode:addNode("Parallel", "stayalive")
  parallel:setCondition("IsSelfHealthLow")
    parallel:addNode("HealSelf", "healself")
end

function defend(parentnode)
  local parallel = parentnode:addNode("Parallel", "defend")
  parallel:setCondition("And(IsAttacked,Filter(SelectAttackers))")
    parallel:addNode("AttackSelection", "attack")
end

function healAlly(parentnode)
  local parallel = parentnode:addNode("Parallel", "healally")
  parallel:setCondition("And(IsAllyHealthLow,Filter(SelectLowHealth))")
    parallel:addNode("HealOther", "healother")
end

function attackAggro(parentnode)
  local parallel = parentnode:addNode("Parallel", "attackaggro")
  parallel:setCondition("Filter(SelectAggro)")
    parallel:addNode("AttackSelection", "attack")
end

function rezzAlly(parentnode)
  local parallel = parentnode:addNode("Parallel", "rezzally")
  parallel:setCondition("Filter(SelectDeadAllies)")
    parallel:addNode("Steer(SelectionSeek)", "follow")
    parallel:addNode("ResurrectSelection", "attack"):setCondition("IsCloseToSelection{1}")
end
