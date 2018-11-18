#pragma once

namespace AI {

/**
 * @ingroup AI
 */
class SelectAggro : public ai::IFilter
{
public:
    FILTER_FACTORY(SelectAggro)
    FILTER_CLASS(SelectAggro)

    void filter(const ai::AIPtr& entity) override;
};

}