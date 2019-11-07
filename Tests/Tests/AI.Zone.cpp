#include "stdafx.h"
#include <catch.hpp>

#include "Loader.h"
#include "Registry.h"
#include "Zone.h"
#include "Agent.h"

TEST_CASE("Zone")
{
    AI::Registry reg;
    reg.Initialize();
    AI::Loader loader(reg);
    // I thought we call a lua function which adds nodes or whatever to the
    // root node, but I'm not sure...
    std::string script = R"lua(
function init(root)
    local node = self:CreateNode("Parallel")
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

}
