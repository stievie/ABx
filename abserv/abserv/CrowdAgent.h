#pragma once

namespace Navigation {

class CrowdAgent
{
    friend class CrowdManager;
private:
    int crowdAgentId_;
    std::weak_ptr<CrowdManager> crowdManager_;
public:
    CrowdAgent();
    ~CrowdAgent();

    bool IsInCrowd() const;
};

}
