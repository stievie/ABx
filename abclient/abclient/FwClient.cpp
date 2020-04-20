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

//#define LOG_ACTIONS

bool VariantMapRead(HashMap<Game::ItemStatIndex, Variant>& vMap, sa::PropReadStream& stream)
{
    vMap.Clear();
    if (stream.GetSize() == 0)
        // Empty but OK
        return true;

    uint16_t count = 0;
    if (!stream.Read<uint16_t>(count))
        return false;

    for (uint16_t i = 0; i < count; ++i)
    {
        uint64_t stat = 0;
        if (!stream.Read<uint64_t>(stat))
            return false;

        uint8_t bt = 0;
        if (!stream.Read<uint8_t>(bt))
            return false;
        VariantType t = static_cast<VariantType>(bt);

        if (t == VAR_NONE || t == VAR_VOIDPTR)
            continue;

        switch (t)
        {
        case VAR_INT:
        {
            int value = 0;
            if (stream.Read<int>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_INT64:
        {
            long long value = 0;
            if (stream.Read<long long>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_BOOL:
        {
            uint8_t value = 0;
            if (stream.Read<uint8_t>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value == 0 ? false : true;
            break;
        }
        case VAR_FLOAT:
        {
            float value = 0.0f;
            if (stream.Read<float>(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = value;
            break;
        }
        case VAR_STRING:
        {
            std::string value;
            if (stream.ReadString(value))
                vMap[static_cast<Game::ItemStatIndex>(stat)] = String(value.c_str(), static_cast<unsigned>(value.length()));
            break;
        }
        default:
            break;
        }
    }
    return true;
}

void VariantMapWrite(const HashMap<Game::ItemStatIndex, Variant>& vMap, sa::PropWriteStream& stream)
{
    stream.Write<uint16_t>(static_cast<uint16_t>(vMap.Size()));
    for (const auto& s : vMap)
    {
        VariantType t = s.second_.GetType();
        if (t == VAR_NONE || t == VAR_VOIDPTR)
            continue;

        stream.Write<uint64_t>(static_cast<uint64_t>(s.first_));
        uint8_t bt = static_cast<uint8_t>(t);
        stream.Write<uint8_t>(bt);
        switch (t)
        {
        case VAR_INT:
            stream.Write<int>(s.second_.GetInt());
            break;
        case VAR_INT64:
            stream.Write<long long>(s.second_.GetInt64());
            break;
        case VAR_BOOL:
            stream.Write<uint8_t>(s.second_.GetBool() ? 1 : 0);
            break;
        case VAR_FLOAT:
            stream.Write<float>(s.second_.GetFloat());
            break;
        case VAR_STRING:
            stream.WriteString(std::string(s.second_.GetString().CString(), static_cast<size_t>(s.second_.GetString().Length())));
            break;
        default:
            break;
        }
    }
}

void LoadStatsFromString(HashMap<Game::ItemStatIndex, Variant>& stats, const std::string& value)
{
    sa::PropReadStream stream;
    stream.Init(value.data(), value.length());
    if (!VariantMapRead(stats, stream))
        URHO3D_LOGERROR("Error loading item stats");
}

void LoadStatsFromString(HashMap<Game::ItemStatIndex, Variant>& stats, const String& value)
{
    return LoadStatsFromString(stats, std::string(value.CString(), static_cast<size_t>(value.Length())));
}

String SaveStatsToString(const HashMap<Game::ItemStatIndex, Variant>& stats)
{
    sa::PropWriteStream stream;
    VariantMapWrite(stats, stream);
    size_t ssize = 0;
    const char* s = stream.GetStream(ssize);
    return String(s, static_cast<unsigned>(ssize));
}

String FwClient::GetProtocolErrorMessage(AB::ErrorCodes err)
{
    switch (err)
    {
    case AB::ErrorCodes::IPBanned:
        return "Your IP Address is banned.";
    case AB::ErrorCodes::TooManyConnectionsFromThisIP:
        return "Too many connection from this IP.";
    case AB::ErrorCodes::InvalidAccountName:
        return "Invalid Account name.";
    case AB::ErrorCodes::InvalidPassword:
        return "Invalid password.";
    case AB::ErrorCodes::NamePasswordMismatch:
        return "Name or password wrong.";
    case AB::ErrorCodes::AlreadyLoggedIn:
        return "You are already logged in.";
    case AB::ErrorCodes::ErrorLoadingCharacter:
        return "Error loading character.";
    case AB::ErrorCodes::AccountBanned:
        return "Your account is banned.";
    case AB::ErrorCodes::WrongProtocolVersion:
        return "Outdated client. Please update the game client.";
    case AB::ErrorCodes::InvalidEmail:
        return "Invalid Email.";
    case AB::ErrorCodes::InvalidAccountKey:
        return "Invalid Account Key.";
    case AB::ErrorCodes::UnknownError:
        return "Internal Error.";
    case AB::ErrorCodes::AccountNameExists:
        return "Login Name already exists.";
    case AB::ErrorCodes::InvalidCharacterName:
        return "Invalid character name.";
    case AB::ErrorCodes::InvalidProfession:
        return "Invalid profession.";
    case AB::ErrorCodes::PlayerNameExists:
        return "Character name already exists.";
    case AB::ErrorCodes::InvalidAccount:
        return "Invalid Account.";
    case AB::ErrorCodes::InvalidPlayerSex:
        return "Invalid character gender.";
    case AB::ErrorCodes::InvalidCharacter:
        return "Invalid character.";
    case AB::ErrorCodes::InvalidCharactersInString:
        return "The string contains invalid characters.";
    case AB::ErrorCodes::NoMoreCharSlots:
        return "You have no free character slots.";
    case AB::ErrorCodes::InvalidGame:
        return "Invalid Game.";
    case AB::ErrorCodes::AllServersFull:
        return "All Servers are full, please try again later.";
    case AB::ErrorCodes::ErrorException:
        return "Exception";
    case AB::ErrorCodes::TokenAuthFailure:
        return "Token authentication failure";
    case AB::ErrorCodes::AccountKeyAlreadyAdded:
        return "This account key was already added to the account";
    default:
        return "";
    }
}

String FwClient::GetSkillErrorMessage(AB::GameProtocol::SkillError err)
{
    switch (err)
    {
    case AB::GameProtocol::SkillError::InvalidSkill:
        return "Invalid Skill";
    case AB::GameProtocol::SkillError::InvalidTarget:
        return "Invalid Skill Target";
    case AB::GameProtocol::SkillError::OutOfRange:
        return "Target out of reach";
    case AB::GameProtocol::SkillError::NoEnergy:
        return "Not enough Energy";
    case AB::GameProtocol::SkillError::NoAdrenaline:
        return "Not enough Adrenaline";
    case AB::GameProtocol::SkillError::Recharging:
        return "Skill is recharging";
    case AB::GameProtocol::SkillError::TargetUndestroyable:
        return "The target is undestroyable";
    case AB::GameProtocol::SkillError::CannotUseSkill:
        return "Can not use skill";
    default:
        return String::EMPTY;
    }
}

String FwClient::GetAttackErrorMessage(AB::GameProtocol::AttackError err)
{
    switch (err)
    {
    case AB::GameProtocol::AttackError::InvalidTarget:
        return "Invalid Target";
    case AB::GameProtocol::AttackError::TargetUndestroyable:
        return "Target is undestroyable";
    case AB::GameProtocol::AttackError::NoTarget:
        return "No Target";
    case AB::GameProtocol::AttackError::TargetObstructed:
        return "Target is obstructed";
    case AB::GameProtocol::AttackError::TargetDodge:
        return "Target dodged";
    case AB::GameProtocol::AttackError::TargetMissed:
        return "Target missed";
    case AB::GameProtocol::AttackError::Interrupted:
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
    case AB::GameProtocol::PlayerErrorValue::InventoryFull:
        return "Your inventory is full";
    case AB::GameProtocol::PlayerErrorValue::ChestFull:
        return "The chest is full";
    case AB::GameProtocol::PlayerErrorValue::TradingPartnerInvalid:
        return "Invalid trading partner";
    case AB::GameProtocol::PlayerErrorValue::TradingPartnerQueueing:
        return "Trading partner is queueing for a match";
    case AB::GameProtocol::PlayerErrorValue::TradingPartnerTrading:
        return "The trading partner is trading with another player";
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
    if (!doc.load_string(strVer.c_str(), static_cast<unsigned>(strVer.length())))
        return;

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
        skill.access = pro.attribute("access").as_uint();
        skill.description = pro.attribute("description").as_string();
        skill.shortDescription = pro.attribute("short_description").as_string();
        skill.icon = pro.attribute("icon").as_string();
        skill.soundEffect = pro.attribute("sound_effect").as_string();
        skill.particleEffect = pro.attribute("particle_effect").as_string();
        skill.professionUuid = pro.attribute("profession").as_string();
        skill.activation = pro.attribute("activation").as_int();
        skill.recharge = pro.attribute("recharge").as_int();
        skill.costEnergy = pro.attribute("const_energy").as_int();
        skill.costEnergyRegen = pro.attribute("const_energy_regen").as_int();
        skill.costAdrenaline = pro.attribute("const_adrenaline").as_int();
        skill.costOvercast = pro.attribute("const_overcast").as_int();
        skill.costHp = pro.attribute("const_hp").as_int();

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
        prof.attributeCount = pro.attribute("num_attr").as_uint();
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
    case Client::State::SelectChar:
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
        currentGameType_ = AB::Entities::GameTypeUnknown;
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

void FwClient::DeleteCharacter(const String& uuid)
{
    if (loggedIn_)
        client_.DeleteCharacter(std::string(uuid.CString()));;
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
        currentGameType_ = AB::Entities::GameTypeUnknown;
        client_.EnterWorld(std::string(charUuid.CString()), std::string(mapUuid.CString()));
    }
}

void FwClient::ChangeWorld(const String& mapUuid)
{
    if (loggedIn_)
    {
        currentGameType_ = AB::Entities::GameTypeUnknown;
        client_.EnterWorld(std::string(currentCharacterUuid_.CString()), std::string(mapUuid.CString()));
    }
}

void FwClient::ChangeMap(const String& mapUuid)
{
    if (loggedIn_)
    {
        currentGameType_ = AB::Entities::GameTypeUnknown;
        client_.ChangeMap(std::string(mapUuid.CString()));
    }
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
    accType_ = AB::Entities::AccountTypeUnknown;
    currentGameType_ = AB::Entities::GameTypeUnknown;
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

void FwClient::SetItemPos(AB::Entities::StoragePlace currentPlace, uint16_t currentPos,
    AB::Entities::StoragePlace place, uint16_t newPos)
{
    if (loggedIn_)
        client_.SetItemPos(currentPlace, currentPos, place, newPos);
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

void FwClient::DepositMoney(uint32_t amount)
{
    if (loggedIn_)
        client_.DepositMoney(amount);
}

void FwClient::WithdrawMoney(uint32_t amount)
{
    if (loggedIn_)
        client_.WithdrawMoney(amount);
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

void FwClient::Command(AB::GameProtocol::CommandType type, const String& data)
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

void FwClient::SetSecondaryProfession(uint32_t profIndex)
{
    if (loggedIn_ && AB::Entities::IsOutpost(currentGameType_))
        client_.SetSecondaryProfession(profIndex);
}

void FwClient::SetAttributeValue(uint32_t attribIndex, uint8_t value)
{
    if (loggedIn_ && AB::Entities::IsOutpost(currentGameType_))
        client_.SetAttributeValue(attribIndex, value);
}

void FwClient::EquipSkill(uint32_t skillIndex, uint8_t pos)
{
    if (loggedIn_ && AB::Entities::IsOutpost(currentGameType_))
        client_.EquipSkill(skillIndex, pos);
}

void FwClient::LoadSkillTemplate(const std::string& templ)
{
    if (loggedIn_ && AB::Entities::IsOutpost(currentGameType_))
        client_.LoadSkillTemplate(templ);
}

void FwClient::TradeRequest(uint32_t targetId)
{
    if (loggedIn_)
        client_.TradeRequest(targetId);
}

void FwClient::TradeCancel()
{
    if (loggedIn_)
        client_.TradeCancel();
}

void FwClient::TradeOffer(uint32_t money, std::vector<uint16_t>&& items)
{
    if (loggedIn_)
        client_.TradeOffer(money, std::forward<std::vector<uint16_t>>(items));
}

void FwClient::TradeAccept()
{
    if (loggedIn_)
        client_.TradeAccept();
}

void FwClient::OnLog(const std::string& message)
{
    String msg(message.c_str(), static_cast<unsigned>(message.length()));
    URHO3D_LOGINFO(msg);
}

void FwClient::OnLoggedIn(const std::string&, const std::string&, AB::Entities::AccountType accType)
{
    accType_ = accType;
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
    LevelManager* lm = GetSubsystem<LevelManager>();
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

void FwClient::OnProtocolError(AB::ErrorCodes err)
{
    LevelManager* lm = GetSubsystem<LevelManager>();
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

void FwClient::OnCharacterDeleted(const std::string& uuid)
{
    auto it = std::find_if(characters_.begin(), characters_.end(), [&uuid](const AB::Entities::Character& current) {
        return current.uuid.compare(uuid) == 0;
    });
    if (it != characters_.end())
        characters_.erase(it);

    using namespace Events::CharacterDeleted;
    VariantMap& eData = GetEventDataMap();
    eData[P_UUID] = String(uuid.c_str());
    SendEvent(Events::E_CHARACTERDELETED, eData);
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
    currentGameType_ = static_cast<AB::Entities::GameType>(packet.gameType);
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
    eData[P_GROUPMASK] = packet.groupMask;
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
        ConcreteItem ci;
        ci.type = static_cast<AB::Entities::ItemType>(item.type);
        ci.index = item.index;
        ci.place = static_cast<AB::Entities::StoragePlace>(item.place);
        ci.pos = item.pos;
        ci.count = item.count;
        ci.value = item.value;
        LoadStatsFromString(ci.stats, item.stats);
        inventory_.push_back(std::move(ci));
    }
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_INVENTORY, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemUpdate& packet)
{
    const auto it = std::find_if(inventory_.begin(), inventory_.end(), [&packet](const ConcreteItem& current) -> bool
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
        LoadStatsFromString(it->stats, packet.stats);
    }
    else
    {
        // Append
        ConcreteItem item;
        item.type = static_cast<AB::Entities::ItemType>(packet.type);
        item.index = packet.index;
        item.place = static_cast<AB::Entities::StoragePlace>(packet.place);
        item.pos = packet.pos;
        item.count = packet.count;
        item.value = packet.value;
        LoadStatsFromString(item.stats, packet.stats);
        inventory_.push_back(std::move(item));
    }

    using namespace Events::InventoryItemUpdate;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_INVENTORYITEMUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::InventoryItemDelete& packet)
{
    auto it = std::find_if(inventory_.begin(), inventory_.end(), [packet](const ConcreteItem& current)
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
        ConcreteItem ci;
        ci.type = static_cast<AB::Entities::ItemType>(item.type);
        ci.index = item.index;
        ci.place = static_cast<AB::Entities::StoragePlace>(item.place);
        ci.pos = item.pos;
        ci.count = item.count;
        ci.value = item.value;
        LoadStatsFromString(ci.stats, item.stats);
        chest_.push_back(std::move(ci));
    }
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_CHEST, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemUpdate& packet)
{
    const auto it = std::find_if(chest_.begin(), chest_.end(), [&packet](const ConcreteItem& current) -> bool
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
        LoadStatsFromString(it->stats, packet.stats);
    }
    else
    {
        // Append
        ConcreteItem item;
        item.type = static_cast<AB::Entities::ItemType>(packet.type);
        item.index = packet.index;
        item.place = static_cast<AB::Entities::StoragePlace>(packet.place);
        item.pos = packet.pos;
        item.count = packet.count;
        item.value = packet.value;
        LoadStatsFromString(item.stats, packet.stats);
        chest_.push_back(std::move(item));
    }

    using namespace Events::ChestItemUpdate;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_ITEMPOS] = packet.pos;
    SendEvent(Events::E_CHESTITEMUPDATE, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ChestItemDelete& packet)
{
    auto it = std::find_if(chest_.begin(), chest_.end(), [packet](const ConcreteItem& current)
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d skill error %d: %s", packet.id, packet.skillIndex, errorMsg.CString());
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d using skill %d: Energy = %d, Adrenaline = %d, Activation = %d, Overcast = %d, HP = %d",
        packet.id, packet.skillIndex, packet.energy, packet.adrenaline, packet.activation, packet.overcast, packet.hp);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %u used skill %u: Recharge = %u", packet.id, packet.skillIndex, packet.recharge);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d pings Target %d, Skill %d", packet.id, packet.targetId, packet.skillIndex);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Effect %d added: Object %d, Ticks = %u", packet.effectIndex, packet.id, packet.ticks);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Effect %d removed: Object %d", packet.effectIndex, packet.id);
#endif
    VariantMap& eData = GetEventDataMap();
    using namespace Events::ObjectEffectRemoved;
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_EFFECTINDEX] = packet.effectIndex;
    QueueEvent(Events::E_OBJECTEFFECTREMOVED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectDamaged& packet)
{
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d was damaged by %d: value %d", packet.id, packet.sourceId, packet.damageValue);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d was healed by %d: HP %d", packet.id, packet.sourceId, packet.healValue);
#endif
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
#ifdef LOG_ACTIONS
    URHO3D_LOGINFOF("Object %d progress type %d, value %d", packet.id, static_cast<int>(packet.type), packet.value);
#endif
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

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectGroupMaskChanged& packet)
{
    using namespace Events::ObjectGroupMaskChanged;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_GROUPMASK] = packet.groupMask;
    QueueEvent(Events::E_OBJECTGROUPMASKCHAGED, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::ObjectSetAttackSpeed& packet)
{
    using namespace Events::ObjectSetAttackSpeed;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_OBJECTID] = packet.id;
    eData[P_SPEED] = static_cast<float>(packet.factor) / 100.0f;
    QueueEvent(Events::E_OBJECTSETATTACKSPEED, eData);
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
    eData[P_TIGGERERID] = packet.triggererId;
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
    eData[P_TIGGERERID] = packet.triggererId;
    eData[P_QUESTS] = q;
    QueueEvent(Events::E_QUESTSELECTIONDIALOGGTRIGGER, eData);
}

void FwClient::OnPacket(int64_t updateTick, const AB::Packets::Server::QuestDialogTrigger& packet)
{
    using namespace Events::QuestDialogTrigger;
    VariantMap& eData = GetEventDataMap();
    eData[P_UPDATETICK] = static_cast<long long>(updateTick);
    eData[P_TIGGERERID] = packet.triggererId;
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

void FwClient::OnPacket(int64_t, const AB::Packets::Server::SetObjectAttributeValue& packet)
{
    using namespace Events::SetAttributeValue;
    VariantMap& eData = GetEventDataMap();
    eData[P_OBJECTID] = packet.objectId;
    eData[P_ATTRIBINDEX] = packet.attribIndex;
    eData[P_VALUE] = packet.value;
    eData[P_REMAINING] = packet.remaining;
    QueueEvent(Events::E_SET_ATTRIBUTEVALUE, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSecProfessionChanged& packet)
{
    using namespace Events::SetSecProfession;
    VariantMap& eData = GetEventDataMap();
    eData[P_OBJECTID] = packet.objectId;
    eData[P_PROFINDEX] = packet.profIndex;
    QueueEvent(Events::E_SET_SECPROFESSION, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::ObjectSetSkill& packet)
{
    using namespace Events::SetSkill;
    VariantMap& eData = GetEventDataMap();
    eData[P_OBJECTID] = packet.objectId;
    eData[P_SKILLINDEX] = packet.skillIndex;
    eData[P_SKILLPOS] = packet.pos;
    QueueEvent(Events::E_SET_SKILL, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::SkillTemplateLoaded& packet)
{
    using namespace Events::LoadSkillTemplate;
    VariantMap& eData = GetEventDataMap();
    eData[P_OBJECTID] = packet.objectId;
    eData[P_TEMPLATE] = String(packet.templ.c_str());
    QueueEvent(Events::E_LOAD_SKILLTEMPLATE, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::TradeDialogTrigger& packet)
{
    using namespace Events::TradeDialogTrigger;
    VariantMap& eData = GetEventDataMap();
    eData[P_SOURDEID] = packet.sourceId;
    eData[P_TARGETID] = packet.targetId;
    QueueEvent(Events::E_TRADEDIALOG_TRIGGER, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::TradeCancel&)
{
    currentPartnerOffer_ = {};
    using namespace Events::TradeCancel;
    VariantMap& eData = GetEventDataMap();
    QueueEvent(Events::E_TRADECANCEL, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::TradeOffer& packet)
{
    currentPartnerOffer_ = packet;
    using namespace Events::TradeOffer;
    VariantMap& eData = GetEventDataMap();
    QueueEvent(Events::E_TRADEOFFER, eData);
}

void FwClient::OnPacket(int64_t, const AB::Packets::Server::TradeAccepted&)
{
    currentPartnerOffer_ = {};
    using namespace Events::TradeAccepted;
    VariantMap& eData = GetEventDataMap();
    QueueEvent(Events::E_TRADEACCEPTED, eData);
}
