#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#include <AB/ProtocolCodes.h>
#include "Options.h"
#include <Urho3D/ThirdParty/PugiXml/pugixml.hpp>

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
    case AB::Errors::InvalidCharacter:
        return "Invalid character.";
    case AB::Errors::InvalidCharactersInString:
        return "The string contains invalid characters.";
    case AB::Errors::NoMoreCharSlots:
        return "You have no free character slots.";
    case AB::Errors::InvalidGame:
        return "Invalid Game.";
    case AB::Errors::AllServersFull:
        return "All Servers are full, please try again later.";
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
    SubscribeToEvent(AbEvents::E_LEVELREADY, URHO3D_HANDLER(FwClient, HandleLevelReady));
}

FwClient::~FwClient()
{
    client_.receiver_ = nullptr;
    UnsubscribeFromAllEvents();
}

void FwClient::UpdateServers()
{
    client_.GetServers();
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
    using namespace AbEvents::LevelReady;
    String levelName = eventData[P_NAME].GetString();
    levelReady_ = true;
    // Level loaded, send queued events
    for (auto& e : queuedEvents_)
    {
        SendEvent(e.eventId, e.eventData);
    }
    queuedEvents_.Clear();
}

void FwClient::LoadData()
{
    LoadGames();
    LoadSkills();
    LoadAttributes();
    LoadProfessions();
    LoadSkills();
    LoadEffects();
/*    WorkQueue* queue = GetSubsystem<WorkQueue>();
    SharedPtr<WorkItem> item = queue->GetFreeItem();
    item->aux_ = const_cast<FwClient*>(this);
    item->workFunction_ = [](const WorkItem* item, unsigned threadIndex)
    {
        FwClient& self = *(reinterpret_cast<FwClient*>(item->aux_));
        self.LoadSkills();
        self.LoadAttributes();
        self.LoadProfessions();
        self.LoadSkills();
        self.LoadEffects();
    };
    queue->AddWorkItem(item);*/
}

void FwClient::LoadGames()
{
    if (!games_.empty())
        return;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Games.xml");
    if (!file)
    {
        if (!client_.HttpDownload("/_games_", "GameData/Games.xml"))
            return;
    }
    file = cache->GetResource<XMLFile>("Games.xml");
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("games");
    if (!node)
        return;

    for (const auto& pro : node.children("game"))
    {
        AB::Entities::Game game;
        game.uuid = pro.attribute("uuid").as_string();
        game.name = pro.attribute("name").as_string();
        game.type = static_cast<AB::Entities::GameType>(pro.attribute("type").as_uint());
        game.landing = pro.attribute("landing").as_bool();
        games_.emplace(game.uuid, game);
    }
}

void FwClient::LoadSkills()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Skills.xml");
    if (!file)
    {
        if (!client_.HttpDownload("/_skills_", "GameData/Skills.xml"))
            return;
    }
    file = cache->GetResource<XMLFile>("Skills.xml");
    if (!file)
        return;
}

void FwClient::LoadAttributes()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Attributes.xml");
    if (!file)
    {
        if (!client_.HttpDownload("/_attributes_", "GameData/Attributes.xml"))
            return;
    }
    file = cache->GetResource<XMLFile>("Attributes.xml");
    if (!file)
        return;
}

void FwClient::LoadProfessions()
{
    if (!professions_.empty())
        return;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Professions.xml");
    if (!file)
    {
        if (!client_.HttpDownload("/_professions_", "GameData/Professions.xml"))
            return;
    }
    file = cache->GetResource<XMLFile>("Professions.xml");
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("professions");
    if (!node)
        return;

    for (const auto& pro : node.children("prof"))
    {
        AB::Entities::Profession prof;
        prof.uuid = pro.attribute("uuid").as_string();
        prof.index = pro.attribute("index").as_uint();
        prof.name = pro.attribute("name").as_string();
        prof.abbr = pro.attribute("abbr").as_string();
        prof.attributeCount = pro.attribute("num_attr").as_uint();
        for (const auto& attr : pro.children("attr"))
        {
            prof.attributeUuids.push_back(attr.attribute("uuid").as_string());
        }
        professions_.emplace(prof.uuid, prof);
    }
}

void FwClient::LoadEffects()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Effects.xml");
    if (!file)
    {
        if (!client_.HttpDownload("/_effects_", "GameData/Effects.xml"))
            return;
    }
    file = cache->GetResource<XMLFile>("Effects.xml");
    if (!file)
        return;
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

void FwClient::CreatePlayer(const String& name, const String& profUuid, AB::Entities::CharacterSex sex, bool isPvp)
{
    if (loggedIn_)
    {
        client_.CreatePlayer(std::string(name.CString()),
            std::string(profUuid.CString()), sex, isPvp);
    }
}

void FwClient::EnterWorld(const String& charUuid, const String& mapUuid)
{
    if (loggedIn_)
    {
        currentCharacterUuid_ = charUuid;
        client_.EnterWorld(std::string(charUuid.CString()), std::string(mapUuid.CString()));
    }
}

void FwClient::ChangeWorld(const String& mapUuid)
{
    if (loggedIn_)
        client_.EnterWorld(std::string(currentCharacterUuid_.CString()), std::string(mapUuid.CString()));
}

void FwClient::ChangeServer(const String& serverId)
{
    if (loggedIn_)
    {
        auto it = services_.find(std::string(serverId.CString()));
        if (it != services_.end())
        {
            client_.EnterWorld(std::string(currentCharacterUuid_.CString()),
                std::string(currentMapUuid_.CString()),
                (*it).second.host, (*it).second.port);
        }
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

void FwClient::DeleteMail(const std::string& uuid)
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

void FwClient::OnLoggedIn(const std::string&)
{
    LoadData();
}

void FwClient::OnGetCharlist(const AB::Entities::CharacterList& chars)
{
    levelReady_ = false;
    characters_ = chars;
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::SetLevel;
    currentLevel_ = "CharSelectLevel";
    eData[P_NAME] = currentLevel_;
    SendEvent(AbEvents::E_SETLEVEL, eData);
}

void FwClient::OnGetOutposts(const std::vector<AB::Entities::Game>& games)
{
    outposts_.clear();
    for (const auto& game : games)
    {
        outposts_[game.uuid] = game;
    }
}

void FwClient::OnGetServices(const std::vector<AB::Entities::Service>& services)
{
    services_.clear();
    for (const auto & s : services)
        services_[s.uuid] = s;
}

void FwClient::OnGetMailHeaders(int64_t updateTick, const std::vector<AB::Entities::MailHeader>& headers)
{
    mailHeaders_ = headers;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAILINBOX, eData);
}

void FwClient::OnGetMail(int64_t updateTick, const AB::Entities::Mail& mail)
{
    currentMail_ = mail;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAILREAD, eData);
}

void FwClient::OnEnterWorld(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, uint32_t playerId)
{
    levelReady_ = false;
    playerId_ = playerId;
    currentServerId_ = String(serverId.c_str());
    VariantMap& eData = GetEventDataMap();
    currentLevel_ = "OutpostLevel";
    currentMapUuid_ = String(mapUuid.c_str());
    using namespace AbEvents::SetLevel;
    eData[P_UPDATETICK] = updateTick;
    eData[P_MAPUUID] = currentMapUuid_;
    eData[P_NAME] = currentLevel_;
    SendEvent(AbEvents::E_SETLEVEL, eData);

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
        using namespace AbEvents::SetLevel;
        eData[P_NAME] = "LoginLevel";
        SendEvent(AbEvents::E_SETLEVEL, eData);
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
    using namespace AbEvents::ObjectSpawn;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = updateTick;
    eData[P_EXISTING] = existing;
    eData[P_OBJECTID] = id;
    eData[P_POSITION] = Vector3(pos.x, pos.y, pos.z);
    eData[P_ROTATION] = rot;
    eData[P_SCALE] = Vector3(scale.x, scale.y, scale.z);
    String d(data.Buffer(), static_cast<unsigned>(data.GetSize()));
    eData[P_DATA] = d;
    QueueEvent(AbEvents::E_OBJECTSPAWN, eData);
}

void FwClient::OnDespawnObject(int64_t updateTick, uint32_t id)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectDespawn;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    QueueEvent(AbEvents::E_OBJECTDESPAWN, eData);
}

void FwClient::OnObjectPos(int64_t updateTick, uint32_t id, const Vec3& pos)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectPosUpdate;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_POSITION] = Vector3(pos.x, pos.y, pos.z);
    QueueEvent(AbEvents::E_OBJECTPOSUPDATE, eData);
}

void FwClient::OnObjectRot(int64_t updateTick, uint32_t id, float rot, bool manual)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectRotUpdate;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_ROTATION] = rot;
    eData[P_MANUAL] = manual;
    QueueEvent(AbEvents::E_OBJECTROTUPDATE, eData);
}

void FwClient::OnObjectStateChange(int64_t updateTick, uint32_t id, AB::GameProtocol::CreatureState state)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectStateUpdate;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_STATE] = static_cast<unsigned>(state);
    QueueEvent(AbEvents::E_OBJECTSTATEUPDATE, eData);
}

void FwClient::OnAccountCreated()
{
    // After successful account creation login with this credentials
    Login(accountName_, accountPass_);
}

void FwClient::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    EnterWorld(String(uuid.c_str()), String(mapUuid.c_str()));
}

void FwClient::OnObjectSelected(int64_t updateTick, uint32_t sourceId, uint32_t targetId)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectSelected;
    eData[P_UPDATETICK] = updateTick;
    eData[P_SOURCEID] = sourceId;
    eData[P_TARGETID] = targetId;
    QueueEvent(AbEvents::E_OBJECTSELECTED, eData);
}

void FwClient::OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ServerMessage;
    eData[P_UPDATETICK] = updateTick;
    eData[P_MESSAGETYPE] = type;
    eData[P_SENDER] = String(senderName.data(), (int)senderName.length());
    eData[P_DATA] = String(message.data(), (int)message.length());
    QueueEvent(AbEvents::E_SERVERMESSAGE, eData);
}

void FwClient::OnChatMessage(int64_t updateTick, AB::GameProtocol::ChatMessageChannel channel,
    uint32_t senderId, const std::string& senderName, const std::string& message)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ChatMessage;
    eData[P_UPDATETICK] = updateTick;
    eData[P_MESSAGETYPE] = channel;
    eData[P_SENDERID] = senderId;
    eData[P_SENDER] = String(senderName.data(), (int)senderName.length());
    eData[P_DATA] = String(message.data(), (int)message.length());
    QueueEvent(AbEvents::E_CHATMESSAGE, eData);
}
