#include "stdafx.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#include <AB/ProtocolCodes.h>
#include "Options.h"
#include <Urho3D/ThirdParty/PugiXml/pugixml.hpp>
#include <Urho3D/Network/Network.h>
#include <Urho3D/Network/HttpRequest.h>
#include <iostream>
#include <fstream>
#include "ItemsCache.h"
#include "TimeUtils.h"
#include "AudioManager.h"

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
    case AB::Errors::ErrorException:
        return "Exception";
    default:
        return "";
    }
}

String FwClient::GetSkillErrorMessage(AB::GameProtocol::SkillError err)
{
    switch (err)
    {
    case AB::GameProtocol::SkillErrorInvalidSkill:
        return "Invalid Skill";
    case AB::GameProtocol::SkillErrorInvalidTarget:
        return "Invalid Skill Target";
    case AB::GameProtocol::SkillErrorOutOfRange:
        return "Target out of reach";
    case AB::GameProtocol::SkillErrorNoEnergy:
        return "Not enough Energy";
    case AB::GameProtocol::SkillErrorNoAdrenaline:
        return "Not enough Adrenaline";
    case AB::GameProtocol::SkillErrorRecharging:
        return "Skill is recharging";
    default:
        return String::EMPTY;
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
    SubscribeToEvent(AbEvents::E_LEVELREADY, URHO3D_HANDLER(FwClient, HandleLevelReady));
    SubscibeUpdate();
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

void FwClient::UnsubscribeUpdate()
{
    UnsubscribeFromEvent(E_UPDATE);
}

void FwClient::SubscibeUpdate()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
}

void FwClient::HandleLevelReady(StringHash, VariantMap&)
{
    using namespace AbEvents::LevelReady;
    levelReady_ = true;
    // Level loaded, send queued events
    for (auto& e : queuedEvents_)
    {
        SendEvent(e.eventId, e.eventData);
    }
    queuedEvents_.Clear();
    // Initial update of services
    if (services_.size() == 0)
        UpdateServers();
}

void FwClient::LoadData()
{
    std::stringstream ss;
    if (!client_.HttpRequest("/_versions_", ss))
        return;

    pugi::xml_document doc;
    std::string strVer = ss.str();
    if (!doc.load_string(strVer.c_str(), (unsigned)strVer.length()))
    {
        return;
    }

    const pugi::xml_node& ver_node = doc.child("versions");
    for (const auto& file_node : ver_node.children("version"))
    {
        const pugi::xml_attribute& name_attr = file_node.attribute("name");
        const pugi::xml_attribute& value_attr = file_node.attribute("value");
        std::string strName = name_attr.as_string();
        versions_[String(name_attr.as_string())] = value_attr.as_uint();
    }

    LoadGames(versions_["game_maps"]);
    LoadSkills(versions_["game_skills"]);
    LoadAttributes(versions_["game_attributes"]);
    LoadProfessions(versions_["game_professions"]);
    LoadEffects(versions_["game_effects"]);
    LoadItems(versions_["game_items"]);
    LoadMusic(versions_["game_music"]);
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

bool FwClient::IsOldData(uint32_t curVersion, XMLFile* file)
{
    if (!file)
        return true;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& root = doc->first_child();
    const pugi::xml_attribute& verAttr = root.attribute("version");
    if (verAttr.as_uint() != curVersion)
        return true;
    return false;
}

void FwClient::LoadGames(uint32_t curVersion)
{
    if (!games_.empty())
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Games.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_games_", "GameData/Games.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Games.xml");
    }
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
        game.mapCoordX = pro.attribute("map_coord_x").as_int();
        game.mapCoordY = pro.attribute("map_coord_y").as_int();
        games_.emplace(game.uuid, game);
    }
}

void FwClient::LoadSkills(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Skills.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_skills_", "GameData/Skills.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Skills.xml");
    }
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("skills");
    if (!node)
        return;

    for (const auto& pro : node.children("skill"))
    {
        AB::Entities::Skill skill;
        skill.uuid = pro.attribute("uuid").as_string();
        skill.index = pro.attribute("index").as_uint();
        skill.name = pro.attribute("name").as_string();
        skill.attributeUuid = pro.attribute("attribute").as_string();
        skill.type = static_cast<AB::Entities::SkillType>(pro.attribute("type").as_uint());
        skill.isElite = pro.attribute("elite").as_bool();
        skill.description = pro.attribute("description").as_string();
        skill.shortDescription = pro.attribute("short_description").as_string();
        skill.icon = pro.attribute("icon").as_string();
        skill.soundEffect = pro.attribute("sound_effect").as_string();
        skill.particleEffect = pro.attribute("particle_effect").as_string();

        skills_.emplace(skill.index, skill);
    }
}

void FwClient::LoadAttributes(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Attributes.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_attributes_", "GameData/Attributes.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Attributes.xml");
    }
    if (!file)
        return;
}

void FwClient::LoadProfessions(uint32_t curVersion)
{
    if (!professions_.empty())
        return;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Professions.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_professions_", "GameData/Professions.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Professions.xml");
    }
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
        prof.modelIndexFemale = pro.attribute("model_index_female").as_uint();
        prof.modelIndexMale = pro.attribute("model_index_male").as_uint();
        for (const auto& attr : pro.children("attr"))
        {
            prof.attributeUuids.push_back(attr.attribute("uuid").as_string());
        }
        professions_.emplace(prof.uuid, prof);
    }
}

void FwClient::LoadEffects(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Effects.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_effects_", "GameData/Effects.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Effects.xml");
    }
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("effects");
    if (!node)
        return;

    for (const auto& pro : node.children("effect"))
    {
        AB::Entities::Effect effect;
        effect.uuid = pro.attribute("uuid").as_string();
        effect.index = pro.attribute("index").as_uint();
        effect.name = pro.attribute("name").as_string();
        effect.category = static_cast<AB::Entities::EffectCategory>(pro.attribute("category").as_uint());
        effect.icon = pro.attribute("icon").as_string();
        effect.soundEffect = pro.attribute("sound_effect").as_string();
        effect.particleEffect = pro.attribute("particle_effect").as_string();

        effects_.emplace(effect.index, effect);
    }
}

void FwClient::LoadItems(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Items.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_items_", "GameData/Items.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Items.xml");
    }
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("items");
    if (!node)
        return;

    ItemsCache* items = GetSubsystem<ItemsCache>();
    for (const auto& itm : node.children("item"))
    {
        auto item = items->Create();

        item->uuid_ = itm.attribute("uuid").as_string();
        item->index_ = itm.attribute("index").as_uint();
        item->name_ = itm.attribute("name").as_string();
        item->type_ = static_cast<AB::Entities::ItemType>(itm.attribute("type").as_uint());
        item->modelFile_ = itm.attribute("model").as_string();
        item->iconFile_ = itm.attribute("icon").as_string();

        items->Add(item);
    }
}

void FwClient::LoadMusic(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Music.xml");

    if (!file || IsOldData(curVersion, file))
    {
        if (!client_.HttpDownload("/_music_", "GameData/Music.xml"))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Music.xml");
    }
    if (!file)
        return;

    AudioManager* am = GetSubsystem<AudioManager>();
    am->LoadMusic(file);
}

bool FwClient::MakeHttpRequest(const String& path, const String& outFile)
{
    // No SSL :(
    Urho3D::Network* network = GetSubsystem<Urho3D::Network>();
    std::stringstream ss;
    ss << "https://";
    ss << client_.fileHost_;
    ss << ":" << static_cast<int>(client_.filePort_);
    ss << path.CString();
    String url(ss.str().c_str());
    Vector<String> headers;
    std::stringstream hss;
    hss << "Auth: " << client_.accountUuid_ << client_.password_;
    headers.Push(hss.str().c_str());

    std::remove(outFile.CString());
    std::ofstream f;
    f.open(outFile.CString());
    if (!f.is_open())
        return false;

    SharedPtr<Urho3D::HttpRequest> request = network->MakeHttpRequest(url, "GET", headers);
    for (;;)
    {
        if (request->GetState() == HTTP_INITIALIZING)
        {
            Time::Sleep(5);
            continue;
        }
        if (request->GetState() == HTTP_ERROR)
        {
            URHO3D_LOGERRORF("HTTP request error: %s", request->GetError().CString());
            return false;
        }

        if (request->GetAvailableSize() > 0)
            f << request->ReadBuffer().Buffer();
        else
            break;
    }

    return true;
}

void FwClient::Update(float timeStep)
{
    client_.Update(static_cast<int>(timeStep * 1000));
    if (lastState_ == client_.state_)
        return;

    switch (client_.state_)
    {
    case Client::Client::ClientState::SelectChar:
        loggedIn_ = true;
        break;
    case Client::Client::ClientState::World:
        break;
    }
    lastState_ = client_.state_;
}

void FwClient::HandleUpdate(StringHash, VariantMap& eventData)
{
    using namespace Update;
    float timeStep = eventData[P_TIMESTEP].GetFloat();
    Update(timeStep);
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

void FwClient::CreatePlayer(const String& name, const String& profUuid, uint32_t modelIndex,
    AB::Entities::CharacterSex sex, bool isPvp)
{
    if (loggedIn_)
    {
        client_.CreatePlayer(std::string(name.CString()),
            std::string(profUuid.CString()), modelIndex, sex, isPvp);
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

void FwClient::ChangeMap(const String& mapUuid)
{
    if (loggedIn_)
        client_.ChangeMap(std::string(mapUuid.CString()));
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
    client_.Poll();
    client_.Run();
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

void FwClient::SendMail(const std::string& recipient, const std::string& subject, const std::string& body)
{
    if (loggedIn_)
        client_.SendMail(recipient, subject, body);
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

void FwClient::ClickObject(uint32_t sourceId, uint32_t targetId)
{
    if (loggedIn_)
        client_.ClickObject(sourceId, targetId);
}

void FwClient::SelectObject(uint32_t sourceId, uint32_t targetId)
{
    if (loggedIn_)
        client_.SelectObject(sourceId, targetId);
}

void FwClient::Command(AB::GameProtocol::CommandTypes type, const String& data)
{
    if (loggedIn_)
        client_.Command(type, std::string(data.CString(), data.Length()));
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

void FwClient::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (loggedIn_)
        client_.SetPlayerState(newState);
}

void FwClient::UseSkill(uint32_t index)
{
    if (loggedIn_)
        client_.UseSkill(index);
}

void FwClient::Cancel()
{
    if (loggedIn_)
        client_.Cancel();
}

void FwClient::PartyInvitePlayer(uint32_t objectId)
{
    if (loggedIn_)
        client_.PartyInvitePlayer(objectId);
}

void FwClient::PartyKickPlayer(uint32_t objectId)
{
    if (loggedIn_)
        client_.PartyKickPlayer(objectId);
}

void FwClient::PartyAcceptInvite(uint32_t inviterId)
{
    if (loggedIn_)
        client_.PartyAcceptInvite(inviterId);
}

void FwClient::PartyRejectInvite(uint32_t inviterId)
{
    if (loggedIn_)
        client_.PartyRejectInvite(inviterId);
}

void FwClient::PartyGetMembers(uint32_t partyId)
{
    if (loggedIn_)
        client_.PartyGetMembers(partyId);
}

void FwClient::OnLoggedIn(const std::string&)
{
    LoadData();
}

void FwClient::OnGetCharlist(const AB::Entities::CharList& chars)
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
    for (const auto& s : services)
        services_[s.uuid] = s;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_GOTSERVICES, eData);
}

void FwClient::OnGetMailHeaders(int64_t, const std::vector<AB::Entities::MailHeader>& headers)
{
    mailHeaders_ = headers;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAILINBOX, eData);
}

void FwClient::OnGetMail(int64_t, const AB::Entities::Mail& mail)
{
    currentMail_ = mail;
    VariantMap& eData = GetEventDataMap();
    SendEvent(AbEvents::E_MAILREAD, eData);
}

void FwClient::OnEnterWorld(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, const std::string& instanceUuid, uint32_t playerId,
    AB::Entities::GameType type, uint8_t partySize)
{
    levelReady_ = false;
    playerId_ = playerId;
    currentServerId_ = String(serverId.c_str());
    const AB::Entities::Game& game = games_[mapUuid];
    VariantMap& eData = GetEventDataMap();
    switch (game.type)
    {
    case AB::Entities::GameType::GameTypeOutpost:
        currentLevel_ = "OutpostLevel";
        break;
    case AB::Entities::GameType::GameTypePvPCombat:
        currentLevel_ = "PvpCombatLevel";
        break;
    case AB::Entities::GameType::GameTypeExploreable:
        // TODO:
        currentLevel_ = "PvpCombatLevel";
        break;
    case AB::Entities::GameType::GameTypeMission:
        // TODO:
        currentLevel_ = "PvpCombatLevel";
        break;
    default:
        URHO3D_LOGERRORF("Unknown game type %d, %s", game.type, mapUuid.c_str());
        return;
    }
    currentMapUuid_ = String(mapUuid.c_str());
    using namespace AbEvents::SetLevel;
    eData[P_UPDATETICK] = updateTick;
    eData[P_MAPUUID] = currentMapUuid_;
    eData[P_NAME] = currentLevel_;
    eData[P_INSTANCEUUID] = String(instanceUuid.c_str());
    eData[P_TYPE] = type;
    eData[P_PARTYSIZE] = partySize;
    SendEvent(AbEvents::E_SETLEVEL, eData);

    Graphics* graphics = GetSubsystem<Graphics>();
    graphics->SetWindowTitle("FW - " + accountName_);
}

void FwClient::OnChangeInstance(int64_t updateTick, const std::string& serverId,
    const std::string& mapUuid, const std::string& instanceUuid, const std::string& charUuid)
{
    if (loggedIn_)
    {
        auto it = services_.find(serverId);
        if (it != services_.end())
        {
            VariantMap& eData = GetEventDataMap();
            using namespace AbEvents::ChangingInstance;
            eData[P_UPDATETICK] = updateTick;
            eData[P_SERVERUUID] = String(serverId.c_str());
            eData[P_MAPUUID] = String(mapUuid.c_str());
            eData[P_INSTANCEUUID] = String(instanceUuid.c_str());
            SendEvent(AbEvents::E_CHANGINGINSTANCE, eData);

            currentCharacterUuid_ = String(charUuid.c_str());
            client_.EnterWorld(charUuid, mapUuid,
                (*it).second.host, (*it).second.port, instanceUuid);
        }
        else
        {
            URHO3D_LOGERRORF("Server %s not found", serverId.c_str());
        }
    }
}

void FwClient::OnNetworkError(const std::error_code& err)
{
    loggedIn_ = false;
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    BaseLevel* cl = lm->GetCurrentLevel<BaseLevel>();
    if (cl)
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
    BaseLevel* cl = lm->GetCurrentLevel<BaseLevel>();
    if (cl)
        cl->OnProtocolError(err);
}

void FwClient::OnServerJoined(const AB::Entities::Service& service)
{
    services_[service.uuid] = service;
    using namespace AbEvents::ServerJoined;
    VariantMap& eData = GetEventDataMap();
    eData[P_SERVERID] = String(service.uuid.c_str());
    QueueEvent(AbEvents::E_SERVERJOINED, eData);
}

void FwClient::OnServerLeft(const AB::Entities::Service& service)
{
    auto it = services_.find(service.uuid);
    if (it != services_.end())
        services_.erase(service.uuid);

    using namespace AbEvents::ServerLeft;
    VariantMap& eData = GetEventDataMap();
    eData[P_SERVERID] = String(service.uuid.c_str());
    QueueEvent(AbEvents::E_SERVERLEFT, eData);
}

void FwClient::OnSpawnObject(int64_t updateTick, uint32_t id, const Client::ObjectSpawn& objectSpawn,
    PropReadStream& data, bool existing)
{
    using namespace AbEvents::ObjectSpawn;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = updateTick;
    eData[P_EXISTING] = existing;
    eData[P_OBJECTID] = id;
    eData[P_POSITION] = Vector3(objectSpawn.pos.x, objectSpawn.pos.y, objectSpawn.pos.z);
    eData[P_ROTATION] = objectSpawn.rot;
    eData[P_UNDESTROYABLE] = objectSpawn.undestroyable;
    eData[P_STATE] = static_cast<uint32_t>(objectSpawn.state);
    eData[P_SPEEDFACTOR] = objectSpawn.speed;
    eData[P_GROUPID] = objectSpawn.groupId;
    eData[P_GROUPPOS] = objectSpawn.groupPos;
    eData[P_SCALE] = Vector3(objectSpawn.scale.x, objectSpawn.scale.y, objectSpawn.scale.z);
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

void FwClient::OnObjectSpeedChange(int64_t updateTick, uint32_t id, float speedFactor)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectSpeedUpdate;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_SPEEDFACTOR] = speedFactor;
    QueueEvent(AbEvents::E_OBJECTSPEEDUPDATE, eData);
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

void FwClient::OnObjectSkillFailure(int64_t updateTick, uint32_t id, int skillIndex, AB::GameProtocol::SkillError error)
{
    String errorMsg = GetSkillErrorMessage(error);
    URHO3D_LOGINFOF("Object %d skill error %d: %s", id, skillIndex, errorMsg.CString());
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::SkillFailure;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_SKILLINDEX] = skillIndex;
    eData[P_ERROR] = static_cast<uint8_t>(error);
    eData[P_ERRORMSG] = errorMsg;
    QueueEvent(AbEvents::E_SKILLFAILURE, eData);
}

void FwClient::OnObjectUseSkill(int64_t updateTick, uint32_t id, int skillIndex,
    uint16_t energy, uint16_t adrenaline, uint16_t activation, uint16_t overcast)
{
    URHO3D_LOGINFOF("Object %d using skill %d: Energy = %d, Adrenaline = %d, Activation = %d, Overcast = %d",
        id, skillIndex, energy, adrenaline, activation, overcast);
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectUseSkill;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_SKILLINDEX] = skillIndex;
    eData[P_ENERGY] = energy;
    eData[P_ADRENALINE] = adrenaline;
    eData[P_ACTIVATION] = activation;
    eData[P_OVERCAST] = overcast;
    QueueEvent(AbEvents::E_OBJECTUSESKILL, eData);
}

void FwClient::OnObjectEndUseSkill(int64_t updateTick, uint32_t id, int skillIndex, uint16_t recharge)
{
    URHO3D_LOGINFOF("Object %u used skill %u: Recharge = %u", id, skillIndex, recharge);
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectEndUseSkill;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_SKILLINDEX] = skillIndex;
    eData[P_RECHARGE] = recharge;
    QueueEvent(AbEvents::E_OBJECTENDUSESKILL, eData);
}

void FwClient::OnObjectEffectAdded(int64_t updateTick, uint32_t id, uint32_t effectIndex, uint32_t ticks)
{
    URHO3D_LOGINFOF("Effect %d added: Object %d, Ticks = %u", effectIndex, id, ticks);
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectEffectAdded;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_EFFECTINDEX] = effectIndex;
    eData[P_TICKS] = ticks;
    QueueEvent(AbEvents::E_OBJECTEFFECTADDED, eData);
}

void FwClient::OnObjectEffectRemoved(int64_t updateTick, uint32_t id, uint32_t effectIndex)
{
    URHO3D_LOGINFOF("Effect %d removed: Object %d", effectIndex, id);
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectEffectRemoved;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_EFFECTINDEX] = effectIndex;
    QueueEvent(AbEvents::E_OBJECTEFFECTREMOVED, eData);
}

void FwClient::OnResourceChanged(int64_t updateTick, uint32_t id,
    AB::GameProtocol::ResourceType resType, int16_t value)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ObjectResourceChanged;
    eData[P_UPDATETICK] = updateTick;
    eData[P_OBJECTID] = id;
    eData[P_RESTYPE] = static_cast<uint32_t>(resType);
    eData[P_VALUE] = static_cast<int32_t>(value);
    QueueEvent(AbEvents::E_OBJECTRESOURCECHANGED, eData);
}

void FwClient::OnServerMessage(int64_t updateTick, AB::GameProtocol::ServerMessageType type,
    const std::string& senderName, const std::string& message)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::ServerMessage;
    eData[P_UPDATETICK] = updateTick;
    eData[P_MESSAGETYPE] = type;
    eData[P_SENDER] = String(senderName.data(), static_cast<int>(senderName.length()));
    eData[P_DATA] = String(message.data(), static_cast<int>(message.length()));
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
    eData[P_SENDER] = String(senderName.data(), static_cast<int>(senderName.length()));
    eData[P_DATA] = String(message.data(), static_cast<int>(message.length()));
    QueueEvent(AbEvents::E_CHATMESSAGE, eData);
}

void FwClient::OnPartyInvited(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::PartyInvited;
    eData[P_UPDATETICK] = updateTick;
    eData[P_SOURCEID] = sourceId;
    eData[P_TARGETID] = targetId;
    eData[P_PARTYID] = partyId;
    QueueEvent(AbEvents::E_PARTYINVITED, eData);
}

void FwClient::OnPartyRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::PartyRemoved;
    eData[P_UPDATETICK] = updateTick;
    eData[P_SOURCEID] = sourceId;
    eData[P_TARGETID] = targetId;
    eData[P_PARTYID] = partyId;
    QueueEvent(AbEvents::E_PARTYREMOVED, eData);
}

void FwClient::OnPartyAdded(int64_t updateTick, uint32_t acceptorId, uint32_t leaderId, uint32_t partyId)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::PartyAdded;
    eData[P_UPDATETICK] = updateTick;
    eData[P_PLAYERID] = acceptorId;
    eData[P_LEADERID] = leaderId;
    eData[P_PARTYID] = partyId;
    QueueEvent(AbEvents::E_PARTYADDED, eData);
}

void FwClient::OnPartyInviteRemoved(int64_t updateTick, uint32_t sourceId, uint32_t targetId, uint32_t partyId)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::PartyInviteRemoved;
    eData[P_UPDATETICK] = updateTick;
    eData[P_SOURCEID] = sourceId;
    eData[P_TARGETID] = targetId;
    eData[P_PARTYID] = partyId;
    QueueEvent(AbEvents::E_PARTYINVITEREMOVED, eData);
}

void FwClient::OnPartyInfoMembers(uint32_t partyId, const std::vector<uint32_t>& members)
{
    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::PartyInfoMembers;
    eData[P_PARTYID] = partyId;
    VariantVector _members;
    _members.Resize(static_cast<unsigned>(members.size()));
    for (unsigned i = 0; i < static_cast<unsigned>(members.size()); i++)
        _members[i] = members[i];
    eData[P_MEMBERS] = _members;
    QueueEvent(AbEvents::E_PARTYINFOMEMBERS, eData);
}
