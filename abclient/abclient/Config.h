#pragma once

#undef URHO3D_ANGELSCRIPT
#undef URHO3D_DATABASE
#undef URHO3D_LUA
#undef URHO3D_NETWORK

#define SERVER_LOGIN_HOST "pc.home.lan"
#define SERVER_LOGIN_PORT 2748

#define MAX_CHAT_MESSAGE 120

// Turn Urho3D logging on/off
#define AB_CLIENT_LOGGING

// Animate player head with mouse look
//#define PLAYER_HEAD_ANIMATION