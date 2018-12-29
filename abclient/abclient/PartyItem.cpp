#include "stdafx.h"
#include "PartyItem.h"

void PartyItem::RegisterObject(Context* context)
{
    context->RegisterFactory<PartyItem>();
}

PartyItem::PartyItem(Context* context) :
    HealthBar(context),
    type_(MemberType::Invitee)
{
    SetEnabled(true);
}

PartyItem::~PartyItem()
{
}
