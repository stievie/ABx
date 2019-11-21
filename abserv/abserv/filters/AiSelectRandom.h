#pragma once

#include "Filter.h"

namespace AI {
namespace Filters {

class SelectRandom : public Filter
{
    FILTER_CLASS(SelectRandom)
protected:
    uint32_t count_;
public:
    explicit SelectRandom(const ArgumentsType& arguments);
    void Execute(Agent& agent) override;
};

}
}
