#pragma once

using namespace Urho3D;

/// User defined events
namespace AbEvents
{
    /// Load next level
    static const StringHash E_SET_LEVEL = StringHash("Set levels");
    static const StringHash E_LEVEL_READY = StringHash("Level Ready");
    static const StringHash E_OBJECT_SPAWN = StringHash("Object Spawn");
    static const StringHash E_OBJECT_SPAWN_EXISTING = StringHash("Object Spawn existing");
    static const StringHash E_OBJECT_DESPAWN = StringHash("Object Despawn");
    static const StringHash E_OBJECT_POS_UPDATE = StringHash("Object Pos Update");
    static const StringHash E_OBJECT_ROT_UPDATE = StringHash("Object Rot Update");
    static const StringHash E_OBJECT_SATE_UPDATE = StringHash("Object Sate Update");
    static const StringHash E_OBJECT_SELECTED = StringHash("Object Selected");
    static const StringHash E_SERVER_MESSAGE = StringHash("Server Message");
    static const StringHash E_CHAT_MESSAGE = StringHash("Chat Message");
    static const StringHash E_MAIL_INBOX = StringHash("Mail Inbox");
    static const StringHash E_MAIL_READ = StringHash("Mail Read");

    static const StringHash ED_MAP_NAME = StringHash("Map name");

    static const StringHash ED_UPDATE_TICK = StringHash("Update Tick");
    static const StringHash ED_OBJECT_ID = StringHash("Object ID");
    static const StringHash ED_OBJECT_ID2 = StringHash("Object ID 2");
    static const StringHash ED_OBJECT_DATA = StringHash("Object Data");
    static const StringHash ED_POS = StringHash("Position");
    static const StringHash ED_ROTATION = StringHash("Rotation");
    static const StringHash ED_ROTATION_MANUAL = StringHash("Rotation Manual");
    static const StringHash ED_SCALE = StringHash("Scale");
    static const StringHash ED_OBJECT_STATE = StringHash("Object State");
    static const StringHash ED_MESSAGE_TYPE = StringHash("Message Type");
    static const StringHash ED_MESSAGE_DATA = StringHash("Message Data");
    static const StringHash ED_MESSAGE_SENDER = StringHash("Message Sender");
    static const StringHash ED_MESSAGE_SENDER_ID = StringHash("Message Sender ID");
}
