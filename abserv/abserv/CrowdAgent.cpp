#include "stdafx.h"
#include "CrowdAgent.h"
#include <DetourCommon.h>
#include <DetourCrowd.h>

namespace Navigation {

CrowdAgent::CrowdAgent() :
    crowdAgentId_(-1)
{
}

CrowdAgent::~CrowdAgent()
{
}

bool CrowdAgent::IsInCrowd() const
{
    return (crowdManager_.lock()) && crowdAgentId_ != -1;
}

}
