#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include <sstream>
#include <Urho3D/IO/Log.h>

FwClient::FwClient(Context* context) :
    Object(context),
    loggedIn_(false)
{
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
}

void FwClient::Login(const String& name, const String& pass)
{
    if (!loggedIn_)
    {
        accountName_ = name;
        accounbtPass_ = pass;
//        protoLogin_ = Net::ProtocolLogin::Login("localhost", 7171, name, pass, "");
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
    eData[AbEvents::E_SET_LEVEL] = "WorldLevel";
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}
