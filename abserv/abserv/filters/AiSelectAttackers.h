#pragma once

#include <filter/IFilter.h>

namespace AI {

/**
 * @ingroup AI
 */
class SelectAttackers : public ai::IFilter
{
public:
    FILTER_FACTORY(SelectAttackers)
    FILTER_CLASS(SelectAttackers)

    void filter(const ai::AIPtr& entity) override;
};

}
