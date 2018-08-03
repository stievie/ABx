#pragma once

using namespace Urho3D;

/// User defined events
namespace AbEvents
{

URHO3D_EVENT(E_SETLEVEL, SetLevel)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_NAME, Name);        // String
    URHO3D_PARAM(P_MAPUUID, MapUuid);            // String
}

URHO3D_EVENT(E_LEVELREADY, LevelReady)
{
    URHO3D_PARAM(P_NAME, Name);        // String
}

URHO3D_EVENT(E_OBJECTSPAWN, ObjectSpawn)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_EXISTING, Existing);      // bool
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_POSITION, Position);
    URHO3D_PARAM(P_ROTATION, Rotation);
    URHO3D_PARAM(P_SCALE, Scale);
    URHO3D_PARAM(P_STATE, State);            // AB::GameProtocol::CreatureState
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_OBJECTDESPAWN, ObjectDespawn)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
}

URHO3D_EVENT(E_OBJECTPOSUPDATE, ObjectPosUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_POSITION, Position);
}

URHO3D_EVENT(E_OBJECTROTUPDATE, ObjectRotUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_ROTATION, Rotation);
    URHO3D_PARAM(P_MANUAL, Manual);
}

URHO3D_EVENT(E_OBJECTSTATEUPDATE, ObjectStateUpdate)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_STATE, State);
}

URHO3D_EVENT(E_OBJECTSELECTED, ObjectSelected)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);
    URHO3D_PARAM(P_TARGETID, TargetId);
}

URHO3D_EVENT(E_SERVERMESSAGE, ServerMessage)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_MESSAGETYPE, MessageType);
    URHO3D_PARAM(P_SENDER, Sender);
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_CHATMESSAGE, ChatMessage)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_MESSAGETYPE, MessageType);
    URHO3D_PARAM(P_SENDERID, SenderId);
    URHO3D_PARAM(P_SENDER, Sender);
    URHO3D_PARAM(P_DATA, Data);
}

URHO3D_EVENT(E_MAILINBOX, MailInbox)
{
}

URHO3D_EVENT(E_MAILREAD, MailRead)
{
}

URHO3D_EVENT(E_PARTYINVITED, PartyInvited)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
}

URHO3D_EVENT(E_PARTYREMOVED, PartyRemoved)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
}

URHO3D_EVENT(E_PARTYADDED, PartyAdded)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
}

URHO3D_EVENT(E_PARTYINVITEREMOVED, PartyInviteRemoved)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
    URHO3D_PARAM(P_TARGETID, TargetId);     // unit32_t
}

}
