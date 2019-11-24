#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectRandom final : public Filter
{
    FILTER_CLASS(SelectRandom)
private:
    uint32_t count_;
public:
    explicit SelectRandom(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
