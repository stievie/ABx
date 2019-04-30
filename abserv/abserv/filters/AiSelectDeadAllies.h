#pragma once

#include <filter/IFilter.h>

namespace AI {

/**
 * @ingroup AI
 */
class SelectDeadAllies : public ai::IFilter
{
public:
    FILTER_FACTORY(SelectDeadAllies)
    FILTER_CLASS(SelectDeadAllies)

    void filter(const ai::AIPtr& entity) override;
};

}
