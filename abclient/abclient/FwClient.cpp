#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include <sstream>
#include <Urho3D/IO/Log.h>
#include <string>

FwClient::FwClient(Context* context) :
    Object(context),
    loggedIn_(false)
{
    lastState_ = client_.state_;
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
}


FwClient::~FwClient()
{
    UnsubscribeFromAllEvents();
}

bool FwClient::Start()
{
    return true;
}

void FwClient::Stop()
{
    Logout();
}

void FwClient::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (lastState_ == client_.state_)
        return;

    switch (client_.state_)
    {
    case Client::Client::StateSelecChar:
        loggedIn_ = true;
        onSelectCharacter();
        break;
    case Client::Client::StateWorld:
        onEnterGame();
        break;
    }
    lastState_ = client_.state_;
}

void FwClient::Login(const String& name, const String& pass)
{
    if (!loggedIn_)
    {
        accountName_ = name;
        accounbtPass_ = pass;
        client_.Login(std::string(name.CString()), std::string(pass.CString()));
    }
}

void FwClient::Logout()
{
    loggedIn_ = false;
}

void FwClient::onConnectionError(int message)
{
    URHO3D_LOGERROR("onConnectionError()");
}

void FwClient::onConnectionClosed()
{
    URHO3D_LOGINFO("onConnectionClosed()");
}

void FwClient::onEnterGame()
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_SET_LEVEL] = "OutpostLevel";
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}

void FwClient::onSelectCharacter()
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_SET_LEVEL] = "CharSelectLevel";
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}
