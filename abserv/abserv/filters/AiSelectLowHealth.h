#pragma once

namespace AI {

/**
 * @ingroup AI
 */
class SelectLowHealth : public ai::IFilter
{
public:
    FILTER_FACTORY(SelectLowHealth)
    FILTER_CLASS(SelectLowHealth)

    void filter(const ai::AIPtr& entity) override;
};

}