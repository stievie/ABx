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

#include <Urho3DAll.h>

using namespace Urho3D;

namespace Events {

// Shortcuts
URHO3D_EVENT(E_SC_TOGGLEMAP, ToggleMap)
{
}

URHO3D_EVENT(E_SC_MOVEFORWARD, MoveForward)
{
}

URHO3D_EVENT(E_SC_MOVEBACKWARD, MoveBackward)
{
}

URHO3D_EVENT(E_SC_MOVELEFT, MoveLeft)
{
}

URHO3D_EVENT(E_SC_MOVERIGHT, MoveRight)
{
}

URHO3D_EVENT(E_SC_TURNLEFT, TurnLeft)
{
}

URHO3D_EVENT(E_SC_TURNRIGHT, TurnRight)
{
}

URHO3D_EVENT(E_SC_KEEPRUNNING, KeepRunning)
{
}

URHO3D_EVENT(E_SC_REVERSECAMERA, ReverseCamera)
{
}

URHO3D_EVENT(E_SC_HIGHLIGHTOBJECTS, HighlightObjects)
{
}

URHO3D_EVENT(E_SC_MOUSELOOK, MouseLook)
{
}

URHO3D_EVENT(E_SC_TOGGLEPARTYWINDOW, TogglePartyWindow)
{
}

URHO3D_EVENT(E_SC_DEFAULTACTION, DefaultAction)
{
}

URHO3D_EVENT(E_SC_PINGTARGET, PingTarget)
{
}

URHO3D_EVENT(E_SC_TAKESCREENSHOT, TakeScreenshot)
{
}

URHO3D_EVENT(E_SC_TOGGLEMAILWINDOW, ToggleMailWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLENEWMAILWINDOW, ToggleNewMailWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEMISSIONMAPWINDOW, ToggleMissionMapWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEFRIENDLISTWINDOW, ToggleFriendListWindow)
{
}

URHO3D_EVENT(E_SC_REPLYMAIL, ReplyMail)
{
    URHO3D_PARAM(P_RECIPIENT, Recipient);     // String
    URHO3D_PARAM(P_SUBJECT, Subject);         // String
    URHO3D_PARAM(P_BODY, Body);               // String
}

URHO3D_EVENT(E_SC_LOGOUT, Logout)
{
}

URHO3D_EVENT(E_SC_SELECTCHARACTER, SelectCharacter)
{
}

URHO3D_EVENT(E_SC_SHOWCREDITS, ShowCredits)
{
}

URHO3D_EVENT(E_SC_TOGGLEOPTIONS, ToggleOptions)
{
}

URHO3D_EVENT(E_SC_TOGGLEMUTEAUDIO, ToggleMuteAudio)
{
}

URHO3D_EVENT(E_SC_EXITPROGRAM, ExitProgram)
{
}

URHO3D_EVENT(E_SC_CHATGENERAL, ChatGeneral)
{
}

URHO3D_EVENT(E_SC_CHATGUILD, ChatGuild)
{
}

URHO3D_EVENT(E_SC_CHATPARTY, ChatParty)
{
}

URHO3D_EVENT(E_SC_CHATTRADE, ChatTrade)
{
}

URHO3D_EVENT(E_SC_CHATWHISPER, ChatWhisper)
{
}

URHO3D_EVENT(E_SC_TOGGLECHATWINDOW, ToggleChatWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEINVENTORYWINDOW, ToggleInventoryWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLESKILLSWINDOW, ToggleSkillsWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEEQUIPWINDOW, ToggleEquipWindow)
{
}

URHO3D_EVENT(E_SC_TOGGLEGUILDWINDOW, ToggleGuildWindow)
{
}

URHO3D_EVENT(E_SC_HIDEUI, HideUI)
{
}

URHO3D_EVENT(E_SC_SELECTSELF, SelectSelf)
{
}

URHO3D_EVENT(E_SC_SELECTTARGET, SelectTarget)
{
}

URHO3D_EVENT(E_SC_SELECTNEXTFOE, SelectNextFoe)
{
}

URHO3D_EVENT(E_SC_SELECTPREVFOE, SelectPrevFoe)
{
}

URHO3D_EVENT(E_SC_SELECTNEXTALLY, SelectNextAlly)
{
}

URHO3D_EVENT(E_SC_SELECTPREVALLY, SelectPrevAlly)
{
}

URHO3D_EVENT(E_SC_USESKILL1, UseSkill1)
{
}

URHO3D_EVENT(E_SC_USESKILL2, UseSkill2)
{
}

URHO3D_EVENT(E_SC_USESKILL3, UseSkill3)
{
}

URHO3D_EVENT(E_SC_USESKILL4, UseSkill4)
{
}

URHO3D_EVENT(E_SC_USESKILL5, UseSkill5)
{
}

URHO3D_EVENT(E_SC_USESKILL6, UseSkill6)
{
}

URHO3D_EVENT(E_SC_USESKILL7, UseSkill7)
{
}

URHO3D_EVENT(E_SC_USESKILL8, UseSkill8)
{
}

URHO3D_EVENT(E_SC_WEAPONSET1, WeaponSet1)
{
}

URHO3D_EVENT(E_SC_WEAPONSET2, WeaponSet2)
{
}

URHO3D_EVENT(E_SC_WEAPONSET3, WeaponSet3)
{
}

URHO3D_EVENT(E_SC_WEAPONSET4, WeaponSet4)
{
}

URHO3D_EVENT(E_SC_CANCEL, Cancel)
{
}

#ifdef DEBUG_HUD
URHO3D_EVENT(E_SC_TOGGLEDEBUGHUD, ToggleDebugHUD)
{
}

URHO3D_EVENT(E_SC_TOGGLECONSOLE, ToggleConsole)
{
}
#endif

}
