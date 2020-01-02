#pragma once

#include "HealthBar.h"

enum class MemberType
{
    Invitee,
    Member,
    Invitation
};

class PartyItem : public HealthBar
{
    URHO3D_OBJECT(PartyItem, HealthBar)
public:
    static void RegisterObject(Context* context);

    PartyItem(Context* context);
    ~PartyItem() override;

    MemberType type_;
};

