#include "stdafx.h"
#include "Item.h"

Item::Item(Context* context) :
    Object(context),
    index_(0),
    type_(AB::Entities::ItemTypeUnknown),
    stackAble_(false)
{
}

Item::~Item()
{
}
