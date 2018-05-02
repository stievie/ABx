#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#include <AB/ProtocolCodes.h>
#include "Options.h"

#include <Urho3D/DebugNew.h>

String FwClient::GetProtocolErrorMessage(uint8_t err)
{
    switch (err)
    {
    case AB::Errors::IPBanned:
        return "Your IP Address is banned.";
    case AB::Errors::TooManyConnectionsFromThisIP:
        return "Too many connection from this IP.";
    case AB::Errors::InvalidAccountName:
        return "Invalid Account name.";
    case AB::Errors::InvalidPassword:
        return "Invalid password.";
    case AB::Errors::NamePasswordMismatch:
        return "Name or password wrong.";
    case AB::Errors::AlreadyLoggedIn:
        return "You are already logged in.";
    case AB::Errors::ErrorLoadingCharacter:
        return "Error loading character.";
    case AB::Errors::AccountBanned:
        return "Your account is banned.";
    case AB::Errors::WrongProtocolVersion:
        return "Outdated client. Please update the game client.";
    case AB::Errors::InvalidEmail:
        return "Invalid Email.";
    case AB::Errors::InvalidAccountKey:
        return "Invalid Account Key.";
    case AB::Errors::UnknownError:
        return "Internal Error.";
    case AB::Errors::AccountNameExists:
        return "Login Name already exists.";
    case AB::Errors::InvalidCharacterName:
        return "Invalid character name.";
    case AB::Errors::InvalidProfession:
        return "Invalid profession.";
    case AB::Errors::PlayerNameExists:
        return "Character name already exists.";
    case AB::Errors::InvalidAccount:
        return "Invalid Account.";
    case AB::Errors::InvalidPlayerSex:
        return "Invalid character gender.";
    case AB::Errors::InvalidCharactersInString:
        return "The string contains invalid characters.";
    case AB::Errors::NoMoreCharSlots:
        return "You have no free character slots.";
    default:
        return "";
    }
}

FwClient::FwClient(Context* context) :
    Object(context),
    loggedIn_(false),
    playerId_(0)
{
    Options* o = context->GetSubsystem<Options>();
    client_.loginHost_ = std::string(o->loginHost_.CString());
    client_.loginPort_ = o->loginPort_;
    client_.receiver_ = this;
    lastState_ = client_.state_;
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
    SubscribeToEvent(AbEvents::E_LEVEL_READY, URHO3D_HANDLER(FwClient, HandleLevelReady));
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

void FwClient::HandleLevelReady(StringHash eventType, VariantMap& eventData)
{
    String levelName = eventData[AbEvents::E_LEVEL_READY].GetString();
    levelReady_ = true;
    // Level loaded, send queued events
    for (auto& e : queuedEvents_)
    {
        SendEvent(e.eventId, e.eventData);
    }
    queuedEvents_.Clear();
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
    case Client::Client::StateSelectChar:
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
        accountPass_ = pass;
        client_.Login(std::string(name.CString()), std::string(pass.CString()));
    }
}

void FwClient::CreateAccount(const String& name, const String& pass, const String& email, const String& accKey)
{
    if (!loggedIn_)
    {
        accountName_ = name;
        accountPass_ = pass;
        client_.CreateAccount(std::string(name.CString()), std::string(pass.CString()),
            std::string(email.CString()), std::string(accKey.CString()));
    }
}

void FwClient::CreatePlayer(const String& name, const String& prof, AB::Entities::CharacterSex sex, bool isPvp)
{
    if (loggedIn_)
    {
        client_.CreatePlayer(std::string(accountName_.CString()), std::string(accountPass_.CString()),
            std::string(name.CString()), std::string(prof.CString()), sex, isPvp);
    }
}

void FwClient::EnterWorld(const String& charUuid, const String& map)
{
    if (loggedIn_)
    {
        currentCharacterUuid_ = charUuid;
        client_.EnterWorld(std::string(charUuid.CString()), std::string(map.CString()));
    }
}

void FwClient::ChangeWorld(const String& map)
{
    if (loggedIn_)
    {
        client_.EnterWorld(std::string(currentCharacterUuid_.CString()), std::string(map.CString()));
    }
}

void FwClient::Logout()
{
    client_.Logout();
    loggedIn_ = false;
}

void FwClient::GetMailHeaders()
{
    if (loggedIn_)
        client_.GetMailHeaders();
}

void FwClient::ReadMail(const std::string& uuid)
{
    if (loggedIn_)
        client_.GetMail(uuid);
}

void FwClient::DeleteMail(const std::string & uuid)
{
    if (loggedIn_)
        client_.DeleteMail(uuid);
}

void FwClient::Move(uint8_t direction)
{
    if (loggedIn_)
        client_.Move(direction);
}

void FwClient::Turn(uint8_t direction)
{
    if (loggedIn_)
        client_.Turn(direction);
}

void FwClient::SetDirection(float rad)
{
    if (loggedIn_)
        client_.SetDirection(rad);
}

void FwClient::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (loggedIn_)
        client_.SelectObject(sourceId, targetId);
}

void FwClient::Command(AB::GameProtocol::CommandTypes type, const String& data)
{
    if (loggedIn_)
        client_.Command(type, std::string(data.CString()));
}

void FwClient::GotoPos(const Vector3& pos)
{
    if (loggedIn_)
        client_.GotoPos({ pos.x_, pos.y_, pos.z_ });
}

void FwClient::FollowObject(uint32_t objectId)
{
    if (loggedIn_)
        client_.FollowObject(objectId);
}

void FwClient::OnGetCharlist(const AB::Entities::CharacterList& chars)
{
    levelReady_ = false;
    characters_ = chars;
    VariantMap& eData = GetEventDataMap();
    currentLevel_ = "CharSelectLevel";
    eData[AbEvents::E_SET_LEVEL] = currentLevel_;
    SendEvent(AbEvents::E_SET_LEVEL, eData);
}

void FwClient::OnGetGamelist(const std::vector<AB::Entities::Game>& games)
{
    games_ = games;
}

void FwClient::OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers)
{
    mailHeaders_ = headers;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAIL_INBOX, eData);
}

void FwClient::OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail)
{
    currentMail_ = mail;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAIL_READ, eData);
}

void FwClient::OnEnterWorld(int64_t updateTick, const std::string& mapName, uint32_t playerId)
{
    levelReady_ = false;
    playerId_ = playerId;
    VariantMap& eData = GetEventDataMap();
    currentLevel_ = "OutpostLevel";
    currentMap_ = String(mapName.c_str());
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_MAP_NAME] = currentMap_;
    eData[AbEvents::E_SET_LEVEL] = currentLevel_;
    SendEvent(AbEvents::E_SET_LEVEL, eData);

    Graphics* graphics = GetSubsystem<Graphics>();
    graphics->SetWindowTitle("FW - " + accountName_);
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

void FwClient::QueueEvent(StringHash eventType, VariantMap& eventData)
{
    if (levelReady_)
        SendEvent(eventType, eventData);
    else
        queuedEvents_.Push({ eventType, eventData });
}

void FwClient::OnProtocolError(uint8_t err)
{
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    BaseLevel* cl = static_cast<BaseLevel*>(lm->GetCurrentLevel());
    cl->OnProtocolError(err);
}

void FwClient::OnSpawnObject(int64_t updateTick, uint32_t id, const Vec3& pos, const Vec3& scale, float rot,
    PropReadStream& data, bool existing)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = id;
    eData[AbEvents::ED_POS] = Vector3(pos.x, pos.y, pos.z);
    eData[AbEvents::ED_ROTATION] = rot;
    eData[AbEvents::ED_SCALE] = Vector3(scale.x, scale.y, scale.z);
    String d(data.Buffer(), static_cast<unsigned>(data.GetSize()));
    eData[AbEvents::ED_OBJECT_DATA] = d;
    if (!existing)
        QueueEvent(AbEvents::E_OBJECT_SPAWN, eData);
    else
        QueueEvent(AbEvents::E_OBJECT_SPAWN_EXISTING, eData);
}

void FwClient::OnDespawnObject(int64_t updateTick, uint32_t id)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = id;
    QueueEvent(AbEvents::E_OBJECT_DESPAWN, eData);
}

void FwClient::OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = id;
    eData[AbEvents::ED_POS] = Vector3(pos.x, pos.y, pos.z);
    QueueEvent(AbEvents::E_OBJECT_POS_UPDATE, eData);
}

void FwClient::OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = id;
    eData[AbEvents::ED_ROTATION] = rot;
    eData[AbEvents::ED_ROTATION_MANUAL] = manual;
    QueueEvent(AbEvents::E_OBJECT_ROT_UPDATE, eData);
}

void FwClient::OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = id;
    eData[AbEvents::ED_OBJECT_STATE] = static_cast<unsigned>(state);
    QueueEvent(AbEvents::E_OBJECT_SATE_UPDATE, eData);
}

void FwClient::OnAccountCreated()
{
    // After successful account creation login with this credentials
    Login(accountName_, accountPass_);
}

void FwClient::OnPlayerCreated(const std::string& name, const std::string& map)
{
    EnterWorld(String(name.c_str()), String(map.c_str()));
}

void FwClient::OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_OBJECT_ID] = sourceId;
    eData[AbEvents::ED_OBJECT_ID2] = targetId;
    QueueEvent(AbEvents::E_OBJECT_SELECTED, eData);
}

void FwClient::OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_MESSAGE_TYPE] = type;
    eData[AbEvents::ED_MESSAGE_SENDER] = String(senderName.data(), (int)senderName.length());
    eData[AbEvents::ED_MESSAGE_DATA] = String(message.data(), (int)message.length());
    QueueEvent(AbEvents::E_SERVER_MESSAGE, eData);
}

void FwClient::OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
    uint32_t senderId, const std::string& senderName, const std::string& message)
{
    VariantMap& eData = GetEventDataMap();
    eData[AbEvents::ED_UPDATE_TICK] = updateTick;
    eData[AbEvents::ED_MESSAGE_TYPE] = channel;
    eData[AbEvents::ED_MESSAGE_SENDER_ID] = senderId;
    eData[AbEvents::ED_MESSAGE_SENDER] = String(senderName.data(), (int)senderName.length());
    eData[AbEvents::ED_MESSAGE_DATA] = String(message.data(), (int)message.length());
    QueueEvent(AbEvents::E_CHAT_MESSAGE, eData);
}
