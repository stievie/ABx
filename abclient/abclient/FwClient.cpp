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
    client_.receiver_ = this;
    lastState_ = client_.state_;
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
}


FwClient::~FwClient()
{
    client_.receiver_ = nullptr;
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
    client_.Update();
    if (lastState_ == client_.state_)
        return;

    switch (client_.state_)
    {
    case Client::Client::StateSelecChar:
        loggedIn_ = true;
        break;
    case Client::Client::StateWorld:
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

void FwClient::EnterWorld(const String& charName)
{
    if (loggedIn_)
    {
        client_.EnterWorld(std::string(charName.CString()));
    }
}

void FwClient::Logout()
{
    loggedIn_ = false;
}

void FwClient::OnGetCharlist()
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_SET_LEVEL] = "CharSelectLevel";
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}

void FwClient::OnEnterWorld(const std::string& mapName)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::E_SET_LEVEL] = "OutpostLevel";
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}

void FwClient::OnNetworkError(const std::error_code& err)
{
}

void FwClient::OnProtocolError(uint8_t err)
{
}
