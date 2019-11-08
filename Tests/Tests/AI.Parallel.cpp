#include "stdafx.h"
#include <catch.hpp>

#include "AI.Mockup.h"
#include "Loader.h"
#include "Registry.h"
#include "Zone.h"
#include "Agent.h"

TEST_CASE("Parallel")
{
    AI::TestRegistry reg;
    reg.Initialize();
    AI::Loader loader(reg);
    const std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Parallel")
    node:AddNode(self:CreateNode("TestAction"))
    root:AddNode(node)
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
    AI::Loader loader(reg);
    const std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Parallel")
    node:AddNode(self:CreateNode("TestAction"))
    node:AddNode(self:CreateNode("RunningAction"))
    root:AddNode(node)
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
    AI::Loader loader(reg);
    const std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Parallel")
    node:AddNode(self:CreateNode("TestAction"))
    node:AddNode(self:CreateNode("Running2Action"))
    root:AddNode(node)
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
