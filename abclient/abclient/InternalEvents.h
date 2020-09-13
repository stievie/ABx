/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

using namespace Urho3D;

/// Client internal events
namespace Events
{

URHO3D_EVENT(E_START_PROGRAM, StartProgram)
{
    URHO3D_PARAM(P_COMMAND, Command);        // String
}

URHO3D_EVENT(E_RESTART, Restart)
{
}

URHO3D_EVENT(E_UPDATESTART, UpdateStart)
{
}

URHO3D_EVENT(E_CANCELUPDATE, CancelUpdate)
{
}

URHO3D_EVENT(E_UPDATEPROGRESS, UpdateProgress)
{
    URHO3D_PARAM(P_VALUE, Value);        // int
    URHO3D_PARAM(P_MAX, Max);        // int
    URHO3D_PARAM(P_PERCENT, Percent);        // float
}

URHO3D_EVENT(E_UPDATEDOWNLOADPROGRESS, UpdateDownloadProgress)
{
    URHO3D_PARAM(P_BYTEPERSEC, BytePerSec);        // int
    URHO3D_PARAM(P_MAX, Max);        // int
    URHO3D_PARAM(P_PERCENT, Percent);        // float
}

URHO3D_EVENT(E_UPDATEFILE, UpdateFile)
{
    URHO3D_PARAM(P_INDEX, Index);        // unsigned
    URHO3D_PARAM(P_COUNT, Count);        // unsigned
    URHO3D_PARAM(P_FILENAME, FileName);        // String
}

URHO3D_EVENT(E_UPDATEDONE, UpdateDone)
{
    URHO3D_PARAM(P_SUCCESS, Success);        // bool
}

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
    URHO3D_PARAM(P_TYPE, Type);        // int
}

URHO3D_EVENT(E_WHISPERTO, WhisperTo)
{
    URHO3D_PARAM(P_NAME, Name);
}

URHO3D_EVENT(E_SENDMAILTO, SendMailTo)
{
    URHO3D_PARAM(P_NAME, Name);
    URHO3D_PARAM(P_SUBJECT, Subject);
    URHO3D_PARAM(P_BODY, Body);
}

URHO3D_EVENT(E_ACTORNAMECLICKED, ActorNameClicked)
{
    URHO3D_PARAM(P_SOURCEID, SourceId);     // unit32_t
}

URHO3D_EVENT(E_ACTORNAMEDOUBLECLICKED, ActorNameDoubleClicked)
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
    URHO3D_PARAM(P_STYLE, Style);     // uint32_t
}

URHO3D_EVENT(E_ACTOR_SKILLS_CHANGED, ActorSkillsChanged)
{
    URHO3D_PARAM(P_OBJECTID, ObjectId);     // uint32_t
    URHO3D_PARAM(P_UPDATEALL, UpdateAll);   // uint32_t
}

URHO3D_EVENT(E_SERVER_PING, ServerPing)
{
    URHO3D_PARAM(P_NAME, Name);
    URHO3D_PARAM(P_HOST, Host);
    URHO3D_PARAM(P_PORT, Port);
    URHO3D_PARAM(P_SUCCESS, Success);
    URHO3D_PARAM(P_PING_TIME, PingTime);
}

}
