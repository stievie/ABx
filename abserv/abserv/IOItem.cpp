#include "stdafx.h"
#include "IOItem.h"
#include "Subsystems.h"
#include "DataClient.h"

namespace IO {

bool IOItem::LoadItem(AB::Entities::Item& item)
{
    IO::DataClient* client = GetSubsystem<IO::DataClient>();
    return client->Read(item);
}

bool IOItem::LoadItemByIndex(AB::Entities::Item& item, uint32_t index)
{
    item.index = index;
    return LoadItem(item);
}

}
