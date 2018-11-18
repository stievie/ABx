function idle(parentnode)
  local prio = parentnode:addNode("PrioritySelector", "walkuncrowded")

  -- if there are too many objects (see parameter) visible of either the same npc type or the max count, leave the area
  -- otherwise walk randomly around in the area around your home position
  --prio:addNode("Steer(WanderAroundHome{100})", "wanderathome"):addCondition("Not(IsCrowded{10, 100})")
  -- if we can't walk in our home base area, we are wandering freely around to find another not crowded area
  prio:addNode("Steer(Wander)", "wanderfreely")
end
