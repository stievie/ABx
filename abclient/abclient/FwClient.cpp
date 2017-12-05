#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include <sstream>
#include <Urho3D/IO/Log.h>
#include <string>
#include "LevelManager.h"
#include "BaseLevel.h"
#include <AB/ProtocolCodes.h>

String FwClient::GetProtocolErrorMessage(uint8_t err)
{
    switch (err)
    {
    case AB::Errors::IPBanned:
        return "Your IP Address is banned";
    case AB::Errors::TooManyConnectionsFromThisIP:
        return "Too many connection from this IP";
    case AB::Errors::InvalidAccountName:
        return "Invalid Account name";
    case AB::Errors::InvalidPassword:
        return "Invalid password";
        break;
    case AB::Errors::NamePasswordMismatch:
        return "Name or password wrong";
    case AB::Errors::AlreadyLoggedIn:
        return "You are already logged in";
    case AB::Errors::ErrorLoadingCharacter:
        return "Error loading character";
    case AB::Errors::AccountBanned:
        return "Your account is banned";
    default:
        return "";
    }
}

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
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    client_.Update(static_cast<int>(timeStep * 1000));
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

void FwClient::EnterWorld(const String& charName, const String& map)
{
    if (loggedIn_)
    {
        client_.EnterWorld(std::string(charName.CString()), std::string(map.CString()));
    }
}

void FwClient::Logout()
{
    client_.Logout();
    loggedIn_ = false;
}

void FwClient::OnGetCharlist(const Client::CharList& chars)
{
    characters_ = chars;
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
    loggedIn_ = false;
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    BaseLevel* cl = static_cast<BaseLevel*>(lm->GetCurrentLevel());

    cl->OnNetworkError(err);

    if (lm->GetLevelName() != "LoginLevel")
    {
        // Disconnect -> Relogin
        VariantMap& eData = GetEventDataMap();
        eData[AbEvents::E_SET_LEVEL] = "LoginLevel";
        SendEvent(AbEvents::E_SET_LEVEL, eData);
    }
}

void FwClient::OnProtocolError(uint8_t err)
{
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    BaseLevel* cl = static_cast<BaseLevel*>(lm->GetCurrentLevel());
    cl->OnProtocolError(err);
}
