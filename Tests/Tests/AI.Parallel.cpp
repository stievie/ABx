#include "stdafx.h"
#include <catch.hpp>

#include "AI.Mockup.h"
#include "LuaLoader.h"
#include "Registry.h"
#include "Zone.h"
#include "Agent.h"

TEST_CASE("Parallel")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
    nd:AddNode(node("TestAction"))
    root:AddNode(nd)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Finished);
}

TEST_CASE("Parallel2")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
    nd:AddNode(node("TestAction"))
    nd:AddNode(node("RunningAction"))
    root:AddNode(nd)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Running);
}

TEST_CASE("Parallel3")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::LuaLoader loader(reg);
    const std::string script = R"lua(
function init(root)
    local nd = node("Parallel")
    nd:AddNode(node("TestAction"))
    nd:AddNode(node("Running2Action"))
    root:AddNode(nd)
end
)lua";
    auto root = loader.LoadString(script);
    REQUIRE(root);

    auto agent = std::make_shared<AI::Agent>(1);
    agent->SetBehavior(root);
    AI::Zone zone("test");
    zone.AddAgent(agent);
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Running);
    // Running2Action needs 2 ticks to complete
    zone.Update(0);
    REQUIRE(agent->GetCurrentStatus() == AI::Node::Status::Finished);
}
