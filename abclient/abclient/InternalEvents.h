#pragma once

using namespace Urho3D;

/// Client internal events
namespace Events
{

URHO3D_EVENT(E_SCREENSHOTTAKEN, ScreenshotTaken)
{
    URHO3D_PARAM(P_FILENAME, Filename);        // String
}

URHO3D_EVENT(E_SETLEVEL, SetLevel)
{
    URHO3D_PARAM(P_UPDATETICK, UpdateTick);
    URHO3D_PARAM(P_NAME, Name);                  // String
    URHO3D_PARAM(P_MAPUUID, MapUuid);            // String
    URHO3D_PARAM(P_INSTANCEUUID, InstanceUuid);  // String
    URHO3D_PARAM(P_TYPE, Type);                  // uint8_t
    URHO3D_PARAM(P_PARTYSIZE, PartySize);        // uint8_t
}

URHO3D_EVENT(E_LEVELREADY, LevelReady)
{
    URHO3D_PARAM(P_NAME, Name);        // String
}

URHO3D_EVENT(E_WHISPERTO, WhisperTo)
{
    URHO3D_PARAM(P_NAME, Name);
}

URHO3D_EVENT(E_SENDMAILTO, SendMailTo)
{
    URHO3D_PARAM(P_NAME, Name);
}

URHO3D_EVENT(E_ACTORNAMECLICKED, ActorNameClicked)
{
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
}

// Audio
URHO3D_EVENT(E_AUDIOPLAY, AudioPlay)
{
    URHO3D_PARAM(P_NAME, Name);     // String
    URHO3D_PARAM(P_TYPE, Type);     // String
}

URHO3D_EVENT(E_AUDIOSTOP, AudioStop)
{
    URHO3D_PARAM(P_NAME, Name);     // String
    URHO3D_PARAM(P_TYPE, Type);     // String
}

URHO3D_EVENT(E_AUDIOSTOPALL, AudioStopAll)
{
}

URHO3D_EVENT(E_AUDIOPLAYMAPMUSIC, AudioPlayMapMusic)
{
    URHO3D_PARAM(P_MAPUUID, MapUuid);     // String
}

/// Play style from current play list
URHO3D_EVENT(E_AUDIOPLAYMUSICSTYLE, AudioPlayMusicStyle)
{
    URHO3D_PARAM(P_STYLE, Style);     // uintt32_t
}

}
