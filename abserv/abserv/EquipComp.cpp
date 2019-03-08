#include "stdafx.h"
#include "EquipComp.h"

namespace Game {
namespace Components {

void EquipComp::Update(uint32_t timeElapsed)
{
    for (const auto& item : items_)
    {
        item.second->Update(timeElapsed);
    }
}

void EquipComp::Equip(ItemPos pos, uint32_t index)
{
}

}
}
