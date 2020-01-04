#include "stdafx.h"
#include "FwClient.h"
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
#include "SkillManager.h"
#include "Shortcuts.h"

//#include <Urho3D/DebugNew.h>

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
    case AB::Errors::TokenAuthFailure:
        return "Token authentication failure";
    case AB::Errors::AccountKeyAlreadyAdded:
        return "This account key was already added to the account";
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
    case AB::GameProtocol::SkillErrorTargetUndestroyable:
        return "The target is undestroyable";
    case AB::GameProtocol::SkillErrorCannotUseSkill:
        return "Can not use skill";
    default:
        return String::EMPTY;
    }
}

String FwClient::GetAttackErrorMessage(AB::GameProtocol::AttackError err)
{
    switch (err)
    {
    case AB::GameProtocol::AttackErrorInvalidTarget:
        return "Invalid Target";
    case AB::GameProtocol::AttackErrorTargetUndestroyable:
        return "Target is undestroyable";
    case AB::GameProtocol::AttackErrorNoTarget:
        return "No Target";
    case AB::GameProtocol::AttackErrorTargetObstructed:
        return "Target is obstructed";
    case AB::GameProtocol::AttackErrorTargetDodge:
        return "Target dodged";
    case AB::GameProtocol::AttackErrorTargetMissed:
        return "Target missed";
    case AB::GameProtocol::AttackErrorInterrupted:
        // Happens when the attack is interrupted, due to some hex or something.
        // No need to show an error message
        return String::EMPTY;
    default:
        return String::EMPTY;
    }
}

String FwClient::GetGameErrorMessage(AB::GameProtocol::PlayerErrorValue err)
{
    switch (err)
    {
    case AB::GameProtocol::PlayerErrorInventoryFull:
        return "Inventory full";
    default:
        return String::EMPTY;
    }
}

FwClient::FwClient(Context* context) :
    Object(context),
    client_(*this)
{
    Options* o = context->GetSubsystem<Options>();
    client_.loginHost_ = std::string(o->loginHost_.CString());
    client_.loginPort_ = o->loginPort_;
    lastState_ = client_.GetState();
    SubscribeToEvent(Events::E_LEVELREADY, URHO3D_HANDLER(FwClient, HandleLevelReady));
    SubscribeUpdate();
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
}

FwClient::~FwClient()
{
    UnsubscribeFromAllEvents();
}

void FwClient::SetEnvironment(const Environment* env)
{
    if (!env)
        return;
    client_.loginHost_ = std::string(env->host.CString());
    client_.loginPort_ = env->port;
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
    PartyLeave();
    Logout();
}

void FwClient::UnsubscribeUpdate()
{
//    URHO3D_LOGINFO("FwClient::UnsubscribeUpdate()");
//    UnsubscribeFromEvent(E_UPDATE);
}

void FwClient::SubscribeUpdate()
{
//    URHO3D_LOGINFO("FwClient::SubscribeUpdate()");
//    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(FwClient, HandleUpdate));
}

void FwClient::HandleLevelReady(StringHash, VariantMap&)
{
    using namespace Events::LevelReady;
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
    LoadProfessions(versions_["game_professions"]);
    LoadAttributes(versions_["game_attributes"]);
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
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_games_", o->GetDataFileStl("GameData/Games.xml")))
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
        game.queueMapUuid = pro.attribute("queue_map").as_string();
        games_.emplace(game.uuid, game);
    }
}

void FwClient::LoadSkills(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Skills.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_skills_", o->GetDataFileStl("GameData/Skills.xml")))
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

    SkillManager* sm = GetSubsystem<SkillManager>();
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

        sm->skills_.emplace(skill.index, skill);
    }
}

void FwClient::LoadAttributes(uint32_t curVersion)
{
    // Professions must be loaded already
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Attributes.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_attributes_", o->GetDataFileStl("GameData/Attributes.xml")))
            return;
        if (file)
            cache->ReloadResource(file);
        file = cache->GetResource<XMLFile>("Attributes.xml");

    }
    if (!file)
        return;

    SkillManager* sm = GetSubsystem<SkillManager>();
    if (sm->professions_.empty())
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("attributes");
    if (!node)
        return;
    for (const auto& pro : node.children("attrib"))
    {
        String profUuid = pro.attribute("profession").as_string();
        String attrName = pro.attribute("name").as_string();
        String attrUuid = pro.attribute("uuid").as_string();
        uint32_t attrIndex = pro.attribute("index").as_uint();
        bool isPrimary = pro.attribute("primary").as_bool();
        sm->attributes_[attrIndex] = {
            std::string(attrUuid.CString()),
            attrIndex,
            std::string(profUuid.CString()),
            std::string(attrName.CString()),
            isPrimary
        };
        AB::Entities::Profession* prof = sm->GetProfession(profUuid);
        if (!prof)
            continue;
        AB::Entities::AttriInfo* attrInfo = sm->GetAttrInfo(*prof, attrUuid);
        if (!attrInfo)
            continue;
        attrInfo->index = attrIndex;
        attrInfo->primary = isPrimary;
    }
}

void FwClient::LoadProfessions(uint32_t curVersion)
{
    SkillManager* sm = GetSubsystem<SkillManager>();
    if (!sm->professions_.empty())
        return;
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Professions.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_professions_", o->GetDataFileStl("GameData/Professions.xml")))
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
            prof.attributes.push_back({ attr.attribute("uuid").as_string(), 0, false });
        }
        sm->professions_.emplace(prof.uuid, prof);
    }
}

void FwClient::LoadEffects(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Effects.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_effects_", o->GetDataFileStl("GameData/Effects.xml")))
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

    SkillManager* sm = GetSubsystem<SkillManager>();
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

        sm->effects_.emplace(effect.index, effect);
    }
}

void FwClient::LoadItems(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Items.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_items_", o->GetDataFileStl("GameData/Items.xml")))
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
        item->objectFile_ = itm.attribute("object").as_string();
        item->iconFile_ = itm.attribute("icon").as_string();
        item->modelClass_ = static_cast<AB::Entities::ModelClass>(itm.attribute("model_class").as_uint());
        item->stackAble_ = itm.attribute("stack_able").as_bool();

        items->Add(item);
    }
}

void FwClient::LoadMusic(uint32_t curVersion)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("Music.xml");

    if (!file || IsOldData(curVersion, file))
    {
        Options* o = GetSubsystem<Options>();
        if (!client_.HttpDownload("/_music_", o->GetDataFileStl("GameData/Music.xml")))
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
    if (lastState_ == client_.GetState())
        return;

    switch (client_.GetState())
    {
    case Client::Client::State::SelectChar:
        loggedIn_ = true;
        break;
    default:
        break;
    }
    lastState_ = client_.GetState();
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

void FwClient::AddAccountKey(const String& newKey)
{
    if (loggedIn_)
    {
        client_.AddAccountKey(std::string(newKey.CString()));
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
    client_.ResetPoll();
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

void FwClient::GetPlayerInfoByName(const std::string& name, uint32_t fields)
{
    if (loggedIn_)
        client_.GetPlayerInfoByName(name, fields);
}

void FwClient::GetPlayerInfoByAccount(const std::string& accountUuid, uint32_t fields)
{
    if (!loggedIn_)
        return;
    const auto it = relatedAccounts_.find(accountUuid);

    if (it != relatedAccounts_.end())
    {
        if (((*it).second.fields & fields) == fields)
        {
            OnPacket(0, (*it).second);
            return;
        }
    }
    client_.GetPlayerInfoByAccount(accountUuid, fields);
}

void FwClient::UpdateInventory()
{
    if (loggedIn_)
        client_.GetInventory();
}

void FwClient::InventoryStoreItem(uint16_t pos)
{
    if (loggedIn_)
        client_.InventoryStoreItem(pos);
}

void FwClient::InventoryDestroyItem(uint16_t pos)
{
    if (loggedIn_)
        client_.InventoryDestroyItem(pos);
}

void FwClient::InventoryDropItem(uint16_t pos)
{
    if (loggedIn_)
        client_.InventoryDropItem(pos);
}

void FwClient::UpdateChest()
{
    if (loggedIn_)
        client_.GetChest();
}

void FwClient::ChestDestroyItem(uint16_t pos)
{
    if (loggedIn_)
        client_.ChestDestroyItem(pos);
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
    {
        auto* sc = GetSubsystem<Shortcuts>();
        client_.FollowObject(objectId, sc->Test(Events::E_SC_PINGTARGET));
    }
}

void FwClient::SetPlayerState(AB::GameProtocol::CreatureState newState)
{
    if (loggedIn_)
        client_.SetPlayerState(newState);
}

void FwClient::UseSkill(uint32_t index)
{
    if (loggedIn_)
    {
        auto* sc = GetSubsystem<Shortcuts>();
        client_.UseSkill(index, sc->Test(Events::E_SC_PINGTARGET));
    }
}

void FwClient::Attack()
{
    if (loggedIn_)
    {
        auto* sc = GetSubsystem<Shortcuts>();
        client_.Attack(sc->Test(Events::E_SC_PINGTARGET));
    }
}

void FwClient::Cancel()
{
    if (loggedIn_)
        client_.Cancel();
}

void FwClient::AddFriend(const String& name, AB::Entities::FriendRelation relation)
{
    if (loggedIn_)
    {
        std::string n(name.Trimmed().CString());
        if (!n.empty())
            client_.AddFriend(n, relation);
    }
}

void FwClient::RemoveFriend(const String& accountUuid)
{
    if (loggedIn_)
        client_.RemoveFriend(std::string(accountUuid.CString(), accountUuid.Length()));
}

void FwClient::UpdateFriendList()
{
    if (loggedIn_)
        client_.UpdateFriendList();
}

void FwClient::SetOnlineStatus(AB::Packets::Server::PlayerInfo::Status status)
{
    if (loggedIn_)
        client_.SetOnlineStatus(status);
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

void FwClient::PartyLeave()
{
    if (loggedIn_)
        client_.PartyLeave();
}

void FwClient::QueueMatch()
{
    if (loggedIn_)
        client_.QueueMatch();
}

void FwClient::UnqueueMatch()
{
    if (loggedIn_)
        client_.UnqueueMatch();
}

void FwClient::OnLog(const std::string& message)
{
    URHO3D_LOGINFO(String(message.c_str()));
}

void FwClient::OnLoggedIn(const std::string&, const std::string&)
{
    LoadData();
}

void FwClient::OnGetCharlist(const AB::Entities::CharList& chars)
{
    levelReady_ = false;
    characters_ = chars;
    VariantMap& eData = GetEventDataMap();
    using namespace Events::SetLevel;
    currentLevel_ = "CharSelectLevel";
    eData[P_NAME] = currentLevel_;
    SendEvent(Events::E_SETLEVEL, eData);
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
    SendEvent(Events::E_GOTSERVICES, eData);
}

void FwClient::OnNetworkError(Client::ConnectionError connectionError, const std::error_code& err)
{
    loggedIn_ = false;
    LevelManager* lm = context_->GetSubsystem<LevelManager>();
    BaseLevel* cl = lm->GetCurrentLevel<BaseLevel>();
    if (cl)
        cl->OnNetworkError(connectionError, err);

    if (lm->GetLevelName() != "LoginLevel")
    {
        // Disconnect -> Relogin
        VariantMap& eData = GetEventDataMap();
        using namespace Events::SetLevel;
        eData[P_NAME] = "LoginLevel";
        SendEvent(Events::E_SETLEVEL, eData);
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

void FwClient::OnAccountCreated()
{
    // After successful account creation login with this credentials
    Login(accountName_, accountPass_);
}

void FwClient::OnPlayerCreated(const std::string& uuid, const std::string& mapUuid)
{
    EnterWorld(String(uuid.c_str()), String(mapUuid.c_str()));
}

void FwClient::OnAccountKeyAdded()
{
    using namespace Events::AccountKeyAdded;
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_ACCOUNTKEYADDED, eData);
}

const std::string& FwClient::GetAccountUuid() const
{
    return client_.accountUuid_;
}

const AB::Packets::Server::PlayerInfo* FwClient::GetRelatedAccount(const String& accountUuid) const
{
    const auto it = relatedAccounts_.find(std::string(accountUuid.CString()));
    if (it == relatedAccounts_.end())
        return nullptr;
    return &(*it).second;
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::ServerJoined& packet)
{
    auto& service = services_[packet.uuid];
    service.type = static_cast<AB::Entities::ServiceType>(packet.type);
    service.uuid = packet.uuid;
    service.host = packet.host;
    service.port = packet.port;
    service.location = packet.location;
    service.name = packet.name;
    using namespace Events::ServerJoined;
    VariantMap& eData = GetEventDataMap();
    eData[P_SERVERID] = String(service.uuid.c_str());
    QueueEvent(Events::E_SERVERJOINED, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::ServerLeft& packet)
{
    auto it = services_.find(packet.uuid);
    if (it != services_.end())
        services_.erase(it);

    using namespace Events::ServerLeft;
    VariantMap& eData = GetEventDataMap();
    eData[P_SERVERID] = String(packet.uuid.c_str());
    QueueEvent(Events::E_SERVERLEFT, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChangeInstance& packet)
{
    if (loggedIn_)
    {
        auto it = services_.find(packet.serverUuid);
        if (it != services_.end())
        {
            VariantMap& eData = GetEventDataMap();
            using namespace Events::ChangingInstance;
            eData[P_UPDATETICK] = static_cast<long long>(updateTick);
            eData[P_SERVERUUID] = String(packet.serverUuid.c_str());
            eData[P_MAPUUID] = String(packet.mapUuid.c_str());
            eData[P_INSTANCEUUID] = String(packet.instanceUuid.c_str());
            SendEvent(Events::E_CHANGINGINSTANCE, eData);

            currentCharacterUuid_ = String(packet.charUuid.c_str());
            client_.EnterWorld(packet.charUuid, packet.mapUuid,
                (*it).second.host, (*it).second.port, packet.instanceUuid);
        }
        else
        {
            URHO3D_LOGERRORF("Server %s not found", packet.serverUuid.c_str());
        }
    }
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::EnterWorld& packet)
{
    {
        using namespace Events::LeaveInstance;
        VariantMap& eData = GetEventDataMap();
        eData[P_UPDATETICK] = static_cast<long long>(updateTick);
        SendEvent(Events::E_LEAVEINSTANCE, eData);
    }

    levelReady_ = false;
    playerId_ = packet.playerId;
    currentServerId_ = String(packet.serverUuid.c_str());
    const AB::Entities::Game& game = games_[packet.mapUuid];
    switch (game.type)
    {
    case AB::Entities::GameType::GameTypeOutpost:
    case AB::Entities::GameType::GameTypeTown:
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
        URHO3D_LOGERRORF("Unknown game type %d, %s", game.type, packet.mapUuid.c_str());
        return;
    }
    URHO3D_LOGINFOF("Switching to level %s", currentLevel_.CString());

    currentMapUuid_ = String(packet.mapUuid.c_str());
    {
        using namespace Events::SetLevel;
        VariantMap& eData = GetEventDataMap();
        eData[P_UPDATETICK] = static_cast<long long>(updateTick);
        eData[P_MAPUUID] = currentMapUuid_;
        eData[P_NAME] = currentLevel_;
        eData[P_INSTANCEUUID] = String(packet.instanceUuid.c_str());
        eData[P_TYPE] = packet.gameType;
        eData[P_PARTYSIZE] = packet.partySize;
        SendEvent(Events::E_SETLEVEL, eData);
    }

    Graphics* graphics = GetSubsystem<Graphics>();
    graphics->SetWindowTitle("FW - " + accountName_);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PlayerAutorun& packet)
{
    using namespace Events::PlayerAutorun;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_AUTORUN] = packet.autorun;
    QueueEvent(Events::E_PLAYERAUTORUN, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawn& packet)
{
    using namespace Events::ObjectSpawn;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_EXISTING] = false;
    eData[P_OBJECTID] = packet.id;
    eData[P_OBJECTTYPE] = static_cast<unsigned>(packet.type);
    eData[P_VALIDFIELDS] = packet.validFields;
    eData[P_POSITION] = Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
    eData[P_ROTATION] = packet.rot;
    eData[P_UNDESTROYABLE] = packet.undestroyable;
    eData[P_SELECTABLE] = packet.selectable;
    eData[P_STATE] = static_cast<uint32_t>(packet.state);
    eData[P_SPEEDFACTOR] = packet.speed;
    eData[P_GROUPID] = packet.groupId;
    eData[P_GROUPPOS] = packet.groupPos;
    eData[P_SCALE] = Vector3(packet.scale[0], packet.scale[1], packet.scale[2]);

    String d(packet.data.c_str(), static_cast<unsigned>(packet.data.length()));
    eData[P_DATA] = d;
    QueueEvent(Events::E_OBJECTSPAWN, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpawnExisting& packet)
{
    using namespace Events::ObjectSpawn;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_EXISTING] = true;
    eData[P_OBJECTID] = packet.id;
    eData[P_OBJECTTYPE] = static_cast<unsigned>(packet.type);
    eData[P_VALIDFIELDS] = packet.validFields;
    eData[P_POSITION] = Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
    eData[P_ROTATION] = packet.rot;
    eData[P_UNDESTROYABLE] = packet.undestroyable;
    eData[P_SELECTABLE] = packet.selectable;
    eData[P_STATE] = static_cast<uint32_t>(packet.state);
    eData[P_SPEEDFACTOR] = packet.speed;
    eData[P_GROUPID] = packet.groupId;
    eData[P_GROUPPOS] = packet.groupPos;
    eData[P_SCALE] = Vector3(packet.scale[0], packet.scale[1], packet.scale[2]);

    String d(packet.data.c_str(), static_cast<unsigned>(packet.data.length()));
    eData[P_DATA] = d;
    QueueEvent(Events::E_OBJECTSPAWN, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::MailHeaders& packet)
{
    mailHeaders_.clear();
    mailHeaders_.reserve(packet.count);
    for (const auto& h : packet.headers)
    {
        mailHeaders_.push_back(
            { h.uuid, h.fromName, h.subject, h.created, h.read }
        );
    }
    std::sort(mailHeaders_.begin(), mailHeaders_.end(), [](const auto& lhs, const auto& rhs)
    {
        return lhs.created - rhs.created;
    });

    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_MAILINBOX, eData);

}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::MailComplete& packet)
{
    currentMail_.fromAccountUuid = packet.fromAccountUuid;
    currentMail_.fromName = packet.fromName;
    currentMail_.toName = packet.toName;
    currentMail_.subject = packet.subject;
    currentMail_.message = packet.body;
    currentMail_.created = packet.created;
    currentMail_.isRead = packet.read;
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_MAILREAD, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDespawn& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectDespawn;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    QueueEvent(Events::E_OBJECTDESPAWN, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPosUpdate& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectPosUpdate;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_POSITION] = Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
    QueueEvent(Events::E_OBJECTPOSUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSpeedChanged& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectSpeedUpdate;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_SPEEDFACTOR] = packet.speed;
    QueueEvent(Events::E_OBJECTSPEEDUPDATE, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::InventoryContent& packet)
{
    inventory_.clear();
    inventory_.reserve(packet.count);
    for (const auto& item : packet.items)
    {
        inventory_.push_back({
            static_cast<AB::Entities::ItemType>(item.type),
            item.index,
            static_cast<AB::Entities::StoragePlace>(item.place),
            item.pos,
            item.count,
            item.value
        });
    }
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_INVENTORY, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet)
{
    const auto it = std::find_if(inventory_.begin(), inventory_.end(), [&packet](const InventoryItem& current) -> bool
    {
        return current.pos == packet.pos;
    });
    if (it != inventory_.end())
    {
        // Replace item at the pos
        it->type = static_cast<AB::Entities::ItemType>(packet.type);
        it->index = packet.index;
        it->place = static_cast<AB::Entities::StoragePlace>(packet.place);
        it->pos = packet.pos;
        it->count = packet.count;
        it->value = packet.value;
    }
    else
    {
        // Append
        inventory_.push_back({
            static_cast<AB::Entities::ItemType>(packet.type),
            packet.index,
            static_cast<AB::Entities::StoragePlace>(packet.place),
            packet.pos,
            packet.count,
            packet.value
        });
    }

    using namespace Events::InventoryItemUpdate;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_INVENTORYITEMUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet)
{
    auto it = std::find_if(inventory_.begin(), inventory_.end(), [packet](const InventoryItem& current)
    {
        return current.pos == packet.pos;
    });
    if (it == inventory_.end())
        return;

    using namespace Events::InventoryItemDelete;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_INVENTORYITEMDELETE, eData);

    inventory_.erase(it);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::ChestContent& packet)
{
    chest_.clear();
    chest_.reserve(packet.count);
    for (const auto& item : packet.items)
    {
        chest_.push_back({
            static_cast<AB::Entities::ItemType>(item.type),
            item.index,
            static_cast<AB::Entities::StoragePlace>(item.place),
            item.pos,
            item.count,
            item.value
        });
    }
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_CHEST, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet)
{
    const auto it = std::find_if(chest_.begin(), chest_.end(), [&packet](const InventoryItem& current) -> bool
    {
        return current.pos == packet.pos;
    });
    if (it != chest_.end())
    {
        // Replace item at the pos
        it->type = static_cast<AB::Entities::ItemType>(packet.type);
        it->index = packet.index;
        it->place = static_cast<AB::Entities::StoragePlace>(packet.place);
        it->pos = packet.pos;
        it->count = packet.count;
        it->value = packet.value;
    }
    else
    {
        // Append
        chest_.push_back({
            static_cast<AB::Entities::ItemType>(packet.type),
            packet.index,
            static_cast<AB::Entities::StoragePlace>(packet.place),
            packet.pos,
            packet.count,
            packet.value
            });
    }

    using namespace Events::ChestItemUpdate;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_CHESTITEMUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet)
{
    auto it = std::find_if(chest_.begin(), chest_.end(), [packet](const InventoryItem& current)
    {
        return current.pos == packet.pos;
    });
    if (it == chest_.end())
        return;

    using namespace Events::ChestItemDelete;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_CHESTITEMDELETE, eData);

    chest_.erase(it);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectRotationUpdate& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectRotUpdate;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_ROTATION] = packet.yRot;
    eData[P_MANUAL] = packet.manual;
    QueueEvent(Events::E_OBJECTROTUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectTargetSelected& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectSelected;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_SOURCEID] = packet.id;
    eData[P_TARGETID] = packet.targetId;
    QueueEvent(Events::E_OBJECTSELECTED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectStateChanged& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectStateUpdate;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_STATE] = static_cast<unsigned>(packet.state);
    QueueEvent(Events::E_OBJECTSTATEUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::GameError& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PlayerError;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ERROR] = static_cast<uint8_t>(packet.code);
    eData[P_ERRORMSG] = GetGameErrorMessage(static_cast<AB::GameProtocol::PlayerErrorValue>(packet.code));
    QueueEvent(Events::E_PLAYERERROR, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillFailure& packet)
{
    String errorMsg = GetSkillErrorMessage(static_cast<AB::GameProtocol::SkillError>(packet.errorCode));
    URHO3D_LOGINFOF("Object %d skill error %d: %s", packet.id, packet.skillIndex, errorMsg.CString());
    VariantMap& eData = GetEventDataMap();
    using namespace Events::SkillFailure;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_SKILLINDEX] = packet.skillIndex;
    eData[P_ERROR] = packet.errorCode;
    eData[P_ERRORMSG] = errorMsg;
    QueueEvent(Events::E_SKILLFAILURE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectUseSkill& packet)
{
    URHO3D_LOGINFOF("Object %d using skill %d: Energy = %d, Adrenaline = %d, Activation = %d, Overcast = %d, HP = %d",
        packet.id, packet.skillIndex, packet.energy, packet.adrenaline, packet.activation, packet.overcast, packet.hp);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectUseSkill;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_SKILLINDEX] = packet.skillIndex;
    eData[P_ENERGY] = packet.energy;
    eData[P_ADRENALINE] = packet.adrenaline;
    eData[P_ACTIVATION] = packet.activation;
    eData[P_OVERCAST] = packet.overcast;
    eData[P_HPCOST] = packet.hp;
    QueueEvent(Events::E_OBJECTUSESKILL, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSkillSuccess& packet)
{
    URHO3D_LOGINFOF("Object %u used skill %u: Recharge = %u", packet.id, packet.skillIndex, packet.recharge);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectEndUseSkill;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_SKILLINDEX] = packet.skillIndex;
    eData[P_RECHARGE] = packet.recharge;
    QueueEvent(Events::E_OBJECTENDUSESKILL, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectAttackFailure& packet)
{
    String errorMsg = GetAttackErrorMessage(static_cast<AB::GameProtocol::AttackError>(packet.errorCode));
    using namespace Events::AttackFailure;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_ERROR] = packet.errorCode;
    eData[P_ERRORMSG] = errorMsg;
    QueueEvent(Events::E_ATTACKFAILURE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectPingTarget& packet)
{
    URHO3D_LOGINFOF("Object %d pings Target %d, Skill %d", packet.id, packet.targetId, packet.skillIndex);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectPingTarget;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_TARGETID] = packet.targetId;
    eData[P_CALLTTYPE] = packet.pingType;
    eData[P_SKILLINDEX] = packet.skillIndex;
    QueueEvent(Events::E_OBJECTPINGTARGET, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectAdded& packet)
{
    URHO3D_LOGINFOF("Effect %d added: Object %d, Ticks = %u", packet.effectIndex, packet.id, packet.ticks);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectEffectAdded;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_EFFECTINDEX] = packet.effectIndex;
    eData[P_TICKS] = packet.ticks;
    QueueEvent(Events::E_OBJECTEFFECTADDED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectEffectRemoved& packet)
{
    URHO3D_LOGINFOF("Effect %d removed: Object %d", packet.effectIndex, packet.id);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectEffectRemoved;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_EFFECTINDEX] = packet.effectIndex;
    QueueEvent(Events::E_OBJECTEFFECTREMOVED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet)
{
    URHO3D_LOGINFOF("Object %d was damaged by %d: value %d", packet.id, packet.sourceId, packet.damageValue);
    using namespace Events::ObjectDamaged;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_DAMAGERID] = packet.sourceId;
    eData[P_INDEX] = packet.index;
    eData[P_DAMAGETYPE] = packet.damageType;
    eData[P_DAMAGEVALUE] = packet.damageValue;
    QueueEvent(Events::E_OBJECTDAMAGED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectHealed& packet)
{
    URHO3D_LOGINFOF("Object %d was healed by %d: HP %d", packet.id, packet.sourceId, packet.healValue);
    using namespace Events::ObjectHealed;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_HEALERID] = packet.sourceId;
    eData[P_INDEX] = packet.index;
    eData[P_HEALVALUE] = packet.healValue;
    QueueEvent(Events::E_OBJECTHEALED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectProgress& packet)
{
    URHO3D_LOGINFOF("Object %d progress type %d, value %d", packet.id, static_cast<int>(packet.type), packet.value);

    using namespace Events::ObjectProgress;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_TYPE] = packet.type;
    eData[P_VALUE] = packet.value;
    QueueEvent(Events::E_OBJECTPROGRESS, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDroppedItem& packet)
{
    using namespace Events::ObjectItemDropped;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_TARGETID] = packet.targetId;
    eData[P_ITEMID] = packet.itemId;
    eData[P_ITEMINDEX] = packet.itemIndex;
    eData[P_COUNT] = packet.count;
    eData[P_VALUE] = packet.value;
    QueueEvent(Events::E_OBJECTITEMDROPPED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetPosition& packet)
{
    using namespace Events::ObjectSetPosition;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_POSITION] = Vector3(packet.pos[0], packet.pos[1], packet.pos[2]);
    QueueEvent(Events::E_OBJECTSETPOSITION, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ServerMessage& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ServerMessage;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_MESSAGETYPE] = packet.type;
    eData[P_SENDER] = String(packet.sender.data(), static_cast<unsigned>(packet.sender.length()));
    eData[P_DATA] = String(packet.data.data(), static_cast<unsigned>(packet.data.length()));
    QueueEvent(Events::E_SERVERMESSAGE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChatMessage& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ChatMessage;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_MESSAGETYPE] = packet.type;
    eData[P_SENDERID] = packet.senderId;
    eData[P_SENDER] = String(packet.sender.data(), static_cast<unsigned>(packet.sender.length()));
    eData[P_DATA] = String(packet.data.data(), static_cast<unsigned>(packet.data.length()));
    QueueEvent(Events::E_CHATMESSAGE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerInvited& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyInvited;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_SOURCEID] = packet.inviterId;
    eData[P_TARGETID] = packet.inviteeId;
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYINVITED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerRemoved& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyRemoved;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_SOURCEID] = packet.leaderId;
    eData[P_TARGETID] = packet.targetId;
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYREMOVED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyPlayerAdded& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyAdded;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_PLAYERID] = packet.acceptorId;
    eData[P_LEADERID] = packet.leaderId;
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYADDED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyInviteRemoved& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyInviteRemoved;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_SOURCEID] = packet.leaderId;
    eData[P_TARGETID] = packet.targetId;
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYINVITEREMOVED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyResigned& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyResigned;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYRESIGNED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::PartyDefeated& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyDefeated;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_PARTYID] = packet.partyId;
    QueueEvent(Events::E_PARTYDEFEATED, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::PartyMembersInfo& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::PartyInfoMembers;
    eData[P_PARTYID] = packet.partyId;
    VariantVector _members;
    _members.Resize(static_cast<unsigned>(packet.count));
    for (unsigned i = 0; i < static_cast<unsigned>(packet.count); ++i)
        _members[i] = packet.members[i];
    eData[P_MEMBERS] = _members;
    QueueEvent(Events::E_PARTYINFOMEMBERS, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectResourceChanged& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectResourceChanged;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_RESTYPE] = packet.type;
    eData[P_VALUE] = packet.value;
    QueueEvent(Events::E_OBJECTRESOURCECHANGED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::DialogTrigger& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::DialogTrigger;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_DIALOGID] = packet.dialogId;
    QueueEvent(Events::E_DIALOGGTRIGGER, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::FriendList& packet)
{
    friendList_ = packet.friends;
    VariantMap& eData = GetEventDataMap();
    using namespace Events::GotFriendList;
    QueueEvent(Events::E_GOT_FRIENDLIST, eData);

}

void FwClient::UpdatePlayer(const AB::Packets::Server::PlayerInfo& player)
{
    auto it = relatedAccounts_.find(player.accountUuid);
    if (it == relatedAccounts_.end())
    {
        relatedAccounts_.emplace(player.accountUuid, player);
        return;
    }

    AB::Packets::Server::PlayerInfo& relAcc = (*it).second;
    if (player.fields & AB::GameProtocol::PlayerInfoFieldName)
    {
        relAcc.nickName = player.nickName;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldName;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldCurrentName)
    {
        relAcc.currentName = player.currentName;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldCurrentName;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldCurrentMap)
    {
        relAcc.currentMap = player.currentMap;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldCurrentMap;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldOnlineStatus)
    {
        relAcc.status = player.status;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldOnlineStatus;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldRelation)
    {
        relAcc.relation = player.relation;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldRelation;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildGuid)
    {
        relAcc.guildUuid = player.guildUuid;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildGuid;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildRole)
    {
        relAcc.guildRole = player.guildRole;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildRole;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildInviteName)
    {
        relAcc.guildInviteName = player.guildInviteName;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildInviteName;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildInvited)
    {
        relAcc.invited = player.invited;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildInvited;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildJoined)
    {
        relAcc.joined = player.joined;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildJoined;
    }
    if (player.fields & AB::GameProtocol::PlayerInfoFieldGuildExpires)
    {
        relAcc.expires = player.expires;
        relAcc.fields |= AB::GameProtocol::PlayerInfoFieldGuildExpires;
    }
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::PlayerInfo& packet)
{
    UpdatePlayer(packet);
    VariantMap& eData = GetEventDataMap();
    using namespace Events::GotPlayerInfo;
    eData[P_ACCOUNTUUID] = String(packet.accountUuid.c_str());
    QueueEvent(Events::E_GOT_PLAYERINFO, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::FriendAdded& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::FriendAdded;
    eData[P_ACCOUNTUUID] = String(packet.accountUuid.c_str());
    eData[P_RELATION] = static_cast<unsigned>(packet.relation);
    QueueEvent(Events::E_FRIENDADDED, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::FriendRemoved& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::FriendRemoved;
    eData[P_ACCOUNTUUID] = String(packet.accountUuid.c_str());
    eData[P_RELATION] = static_cast<unsigned>(packet.relation);
    QueueEvent(Events::E_FRIENDREMOVED, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::GuildInfo& packet)
{
    // TODO:
    (void)packet;
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::GuildMemberList& packet)
{
    guildMembers_ = packet.members;
    VariantMap& eData = GetEventDataMap();
    using namespace Events::GotGuildMembers;
    QueueEvent(Events::E_GOT_GUILDMEMBERS, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestSelectionDialogTrigger& packet)
{
    VariantMap& eData = GetEventDataMap();
    using namespace Events::QuestSelectionDialogTrigger;
    VariantVector q;
    for (auto i : packet.quests)
        q.Push(i);
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_QUESTS] = q;
    QueueEvent(Events::E_QUESTSELECTIONDIALOGGTRIGGER, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet)
{
    using namespace Events::QuestDialogTrigger;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_QUESTINDEX] = packet.questIndex;
    QueueEvent(Events::E_QUESTDIALOGGTRIGGER, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::NpcHasQuest& packet)
{
    using namespace Events::NpcHasQuest;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.npcId;
    eData[P_HASQUEST] = packet.hasQuest;
    QueueEvent(Events::E_NPCHASQUEST, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDeleted& packet)
{
    using namespace Events::QuestDeleted;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_INDEX] = packet.questIndex;
    eData[P_DELETED] = packet.deleted;
    QueueEvent(Events::E_QUESTDELETED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestRewarded& packet)
{
    using namespace Events::QuestRewarded;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_INDEX] = packet.questIndex;
    eData[P_REWARDED] = packet.rewarded;
    QueueEvent(Events::E_QUESTREWARDED, eData);
}
