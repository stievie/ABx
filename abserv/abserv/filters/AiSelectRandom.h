#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectRandom : public Filter
{
protected:
    uint32_t count_;
public:
    FILTER_CLASS(SelectRandom)
    explicit SelectRandom(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
