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

// Most of it taken from the Character Demo

#include "Actor.h"
#include "BaseLevel.h"
#include "ChatFilter.h"
#include "Definitions.h"
#include "FwClient.h"
#include "HealthBar.h"
#include "ItemsCache.h"
#include "ItemStatsUIElement.h"
#include "LevelManager.h"
#include "MathUtils.h"
#include "Shortcuts.h"
#include "SkillManager.h"
#include "WindowManager.h"
#include <AB/Entities/Skill.h>
#include <AB/ProtocolCodes.h>
#include <abshared/AttribAlgos.h>
#include <abshared/Mechanic.h>
#include <abshared/CollisionLayers.h>
#include <Urho3D/Physics/RigidBody.h>

//#include <Urho3D/DebugNew.h>

#define WALK_SPEED_THRESHOLD (0.5f)
#define RUN_ANIM_SPEED(x) (x > 1.0f ? x * 0.8f : x)
// speed / 2 -> walk animation -> playing at normal speed = speed * 2
#define WALK_ANIM_SPEED(x) (x * 2.0f)

Actor::Actor(Context* context) :
    GameObject(context),
    nameLabel_(nullptr),
    animController_(nullptr),
    model_(nullptr),
    selectedObject_(nullptr),
    skills_{}
{
    SetUpdateEventMask(USE_UPDATE);
    SubscribeToEvent(Events::E_CHATMESSAGE, URHO3D_HANDLER(Actor, HandleChatMessage));
    SubscribeToEvent(Events::E_OBJECTUSESKILL, URHO3D_HANDLER(Actor, HandleSkillUse));
    SubscribeToEvent(Events::E_OBJECTENDUSESKILL, URHO3D_HANDLER(Actor, HandleEndSkillUse));
    SubscribeToEvent(Events::E_OBJECTEFFECTADDED, URHO3D_HANDLER(Actor, HandleEffectAdded));
    SubscribeToEvent(Events::E_PARTYADDED, URHO3D_HANDLER(Actor, HandlePartyAdded));
    SubscribeToEvent(Events::E_PARTYREMOVED, URHO3D_HANDLER(Actor, HandlePartyRemoved));
    SubscribeToEvent(Events::E_OBJECTITEMDROPPED, URHO3D_HANDLER(Actor, HandleItemDropped));
    SubscribeToEvent(Events::E_SET_SECPROFESSION, URHO3D_HANDLER(Actor, HandleObjectSecProfessionChange));
    SubscribeToEvent(Events::E_SET_ATTRIBUTEVALUE, URHO3D_HANDLER(Actor, HandleSetAttribValue));
    SubscribeToEvent(Events::E_LOAD_SKILLTEMPLATE, URHO3D_HANDLER(Actor, HandleLoadSkillTemplate));
    SubscribeToEvent(Events::E_SET_SKILL, URHO3D_HANDLER(Actor, HandleSetSkill));
    SubscribeToEvent(Events::E_OBJECTGROUPMASKCHAGED, URHO3D_HANDLER(Actor, HandleGroupMaskChanged));
    SubscribeToEvent(Events::E_OBJECTSETATTACKSPEED, URHO3D_HANDLER(Actor, HandleSetAttackSpeed));
    SubscribeToEvent(Events::E_DROPTARGET_CHANGED, URHO3D_HANDLER(Actor, HandleDropTargetChanged));
}

Actor::~Actor()
{
    UnsubscribeFromAllEvents();
}

void Actor::RegisterObject(Context* context)
{
    context->RegisterFactory<Actor>();
}

Actor* Actor::CreateActor(uint32_t id, Scene* scene,
    const Vector3& position, const Quaternion& rotation, const Vector3& scale,
    AB::GameProtocol::CreatureState state,
    sa::PropReadStream& data)
{
    Node* node = scene->CreateChild(0, LOCAL);
    node->CreateComponent<SmoothedTransform>();
    Actor* result = node->CreateComponent<Actor>();
    result->gameId_ = id;

    result->Unserialize(data);
    result->Init(scene, position, rotation, scale, state);
    result->PlayStateAnimation(0.0f);
    result->SetMoveToPos(position);
    result->SetRotateTo(rotation);

    return result;
}

void Actor::Init(Scene*, const Vector3& position, const Quaternion& rotation, const Vector3& scale,
    AB::GameProtocol::CreatureState state)
{
    if (itemIndex_ != 0)
        LoadObject(itemIndex_, position, rotation, scale);

    // Must be after load model
    animations_[ANIM_RUN] = GetAnimation(ANIM_RUN);
    animations_[ANIM_WALK] = GetAnimation(ANIM_WALK);
    animations_[ANIM_IDLE] = GetAnimation(ANIM_IDLE);
    animations_[ANIM_SIT] = GetAnimation(ANIM_SIT);
    animations_[ANIM_DYING] = GetAnimation(ANIM_DYING);
    animations_[ANIM_CRY] = GetAnimation(ANIM_CRY);
    animations_[ANIM_CASTING] = GetAnimation(ANIM_CASTING);
    animations_[ANIM_TAUNTING] = GetAnimation(ANIM_TAUNTING);
    animations_[ANIM_PONDER] = GetAnimation(ANIM_PONDER);
    animations_[ANIM_WAVE] = GetAnimation(ANIM_WAVE);
    animations_[ANIM_LAUGH] = GetAnimation(ANIM_LAUGH);
    animations_[ANIM_ATTACK] = GetAnimation(ANIM_ATTACK);
    animations_[ANIM_CHEST_OPENING] = GetAnimation(ANIM_CHEST_OPENING);
    animations_[ANIM_CHEST_CLOSING] = GetAnimation(ANIM_CHEST_CLOSING);
    sounds_[SOUND_SKILLFAILURE] = GetSoundEffect(SOUND_SKILLFAILURE);
    sounds_[SOUND_FOOTSTEPS] = GetSoundEffect(SOUND_FOOTSTEPS);
    sounds_[SOUND_DIE] = GetSoundEffect(SOUND_DIE);

    if (modelClass_ == AB::Entities::ModelClass::Aoe)
    {
        ParticleEmitter* pe = node_->GetComponent<ParticleEmitter>(true);
        if (pe)
        {
            pe->SetEmitting(state == AB::GameProtocol::CreatureState::Idle ? false : true);
        }
    }

    creatureState_ = state;
    if (model_)
    {
        model_->SetCastShadows(true);
        model_->SetOccludee(true);
        model_->SetOccluder(false);
        SubscribeToEvent(model_->GetNode(), E_ANIMATIONFINISHED, URHO3D_HANDLER(Actor, HandleAnimationFinished));
    }
}

bool Actor::LoadObject(uint32_t itemIndex, const Vector3& position, const Quaternion& rotation, const Vector3 scale)
{
    ItemsCache* items = GetSubsystem<ItemsCache>();
    SharedPtr<Item> item = items->Get(itemIndex);
    if (!item)
    {
        URHO3D_LOGERRORF("Model Item not found: %d", itemIndex);
        return false;
    }
    XMLFile* object = item->GetObjectResource<XMLFile>();
    if (!object)
    {
        URHO3D_LOGERRORF("Prefab file not found for %s: %s", item->name_.CString(), item->objectFile_.CString());
        return false;
    }

    modelClass_ = item->modelClass_;
    XMLElement root = object->GetRoot();
    unsigned nodeId = root.GetUInt("id");
    SceneResolver resolver;
    Node* adjNode = node_->CreateChild(0, LOCAL);
    resolver.AddNode(nodeId, adjNode);
    adjNode->SetRotation(Quaternion(270, Vector3::UP));
    if (adjNode->LoadXML(root, resolver, true, true))
    {
        resolver.Resolve();
        node_->SetTransform(position, rotation, scale);
        adjNode->ApplyAttributes();
        if (AnimatedModel* animModel = adjNode->GetComponent<AnimatedModel>(true))
        {
            type_ = Actor::Animated;
            animController_ = adjNode->CreateComponent<AnimationController>();
            model_ = animModel;
        }
        else
        {
            type_ = Actor::Static;
            model_ = adjNode->GetComponent<StaticModel>(true);
        }
        if (RigidBody* rigidBody = adjNode->GetComponent<RigidBody>(true))
        {
            // Collide on all layers but camera layer
            rigidBody->SetCollisionMask(Game::ACTOR_COLLISION_LAYER);
            rigidBody->SetCollisionMask(Game::ACTOR_COLLISION_MASK);
        }
        Node* soundSourceNode = node_->CreateChild("SoundSourceNode");
        soundSource_ = soundSourceNode->CreateComponent<SoundSource3D>();
        soundSource_->SetSoundType(SOUND_EFFECT);
        soundSource_->SetNearDistance(2.0f);
        soundSource_->SetFarDistance(15.0f);
    }
    else
    {
        URHO3D_LOGERRORF("Error instantiating prefab %s", item->objectFile_.CString());
        adjNode->Remove();
        return false;
    }
    return true;
}

void Actor::AddModel(uint32_t itemIndex)
{
    if (!model_)
        return;
    ItemsCache* items = GetSubsystem<ItemsCache>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SharedPtr<Item> item = items->Get(itemIndex);
    if (!item)
    {
        URHO3D_LOGERRORF("Model Item not found: %d", itemIndex);
        return;
    }
    XMLFile* xml = item->GetObjectResource<XMLFile>();
    if (!xml)
    {
        URHO3D_LOGERRORF("Prefab file not found: %s", item->objectFile_.CString());
        return;
    }

    XMLElement root = xml->GetRoot();

    Node* node = model_->GetNode();

    SharedPtr<StaticModel> model;

    XMLElement compElem = root.GetChild("component");
    while (compElem)
    {
        String compType = compElem.GetAttribute("type");
        if (compType.Compare("AnimatedModel") == 0)
            model = node->CreateComponent<AnimatedModel>(LOCAL);
        else if (compType.Compare("StaticModel") == 0)
            model = node->CreateComponent<StaticModel>(LOCAL);

        if (model)
        {
            XMLElement attribElem = compElem.GetChild("attribute");
            while (attribElem)
            {
                String attribName = attribElem.GetAttribute("name");
                if (attribName.Compare("Model") == 0)
                {
                    String modelVal = attribElem.GetAttribute("value");
                    StringVector modelVals = modelVal.Split(';');
                    Model* resModel = cache->GetResource<Model>(modelVals[1]);
                    model->SetModel(resModel);
                    model->SetViewMask(model_->GetViewMask());
                }
                else if (attribName.Compare("Material") == 0)
                {
                    String matVal = attribElem.GetAttribute("value");
                    StringVector matVals = matVal.Split(';');
                    for (unsigned i = 1; i < matVals.Size(); ++i)
                    {
                        Material* mat = cache->GetResource<Material>(matVals[i]);
                        model->SetMaterial(i - 1, mat);
                    }
                }
                attribElem = attribElem.GetNext("attribute");
            }

            break;
        }

        compElem = compElem.GetNext("component");
    }
}

void Actor::SetMoveToPos(const Vector3& pos)
{
    auto* smoothTransform = node_->GetComponent<SmoothedTransform>();
    return smoothTransform->SetTargetPosition(pos);
}

const Vector3& Actor::GetMoveToPos() const
{
    auto* smoothTransform = node_->GetComponent<SmoothedTransform>();
    return smoothTransform->GetTargetPosition();
}

void Actor::SetRotateTo(const Quaternion& rot)
{
    auto* smoothTransform = node_->GetComponent<SmoothedTransform>();
    smoothTransform->SetTargetRotation(rot);
}

const Quaternion& Actor::GetRotateTo() const
{
    auto* smoothTransform = node_->GetComponent<SmoothedTransform>();
    return smoothTransform->GetTargetRotation();
}

Vector3 Actor::GetHeadPos() const
{
    Node* headNode = node_->GetChild("Head", true);
    if (headNode)
        return headNode->GetWorldPosition();
    if (model_)
    {
        const BoundingBox& bb = model_->GetBoundingBox();
        Vector3 headPos;
        headPos = node_->GetWorldPosition();
        headPos.y_ += bb.Size().y_;
        return headPos;
    }
    return node_->GetWorldPosition();
}

void Actor::SetUIElementSizePos(UIElement* elem, const IntVector2& size, const IntVector2& pos)
{
    if (size != IntVector2::ZERO)
        elem->SetSize(size);
    if (pos != IntVector2::ZERO)
    {
        elem->SetPosition(pos);
    }
}

bool Actor::IsSpeechBubbleVisible() const
{
    if (speechBubbleWindow_)
        return speechBubbleWindow_->IsVisible();
    return false;
}

void Actor::Update(float timeStep)
{
    Shortcuts* sc = GetSubsystem<Shortcuts>();

    const Vector3& pos = node_->GetPosition();
    Vector3 headPos = GetHeadPos();
    headPos.y_ += 0.5f;
    IntVector2 screenPos = WorldToScreenPoint(pos);
    IntVector2 hpTop = WorldToScreenPoint(headPos);

    const bool highlight = sc->IsTriggered(Events::E_SC_HIGHLIGHTOBJECTS);
    const bool inRange = GetDistanceToPlayer() <= Game::RANGE_SELECT;

    if ((hovered_ || playerSelected_ || highlight || IsSpeechBubbleVisible()) && inRange)
    {
        float sizeFac = 1.0f;
        if (screenPos != IntVector2::ZERO)
        {
            Node* camNode = GetScene()->GetChild("CameraNode");
            if (camNode)
            {
                const Vector3& dist = pos - camNode->GetPosition();
                sizeFac = 10.0f / dist.Length();
            }

            if (nameWindow_)
            {
                IntVector2 labelPos(screenPos.x_ - nameWindow_->GetWidth() / 2, screenPos.y_);
                SetUIElementSizePos(nameWindow_, IntVector2::ZERO, labelPos);
            }

            if (hpBar_)
            {
                IntVector2 ihpPos(screenPos.x_ - hpBar_->GetWidth() / 2, hpTop.y_ - hpBar_->GetHeight() - 5);
                SetUIElementSizePos(hpBar_,
                    IntVector2(static_cast<int>(130.f * sizeFac), static_cast<int>(18.f * sizeFac)),
                    ihpPos);
            }
            else if (classLevel_)
            {
                IntVector2 ihpPos(screenPos.x_ - classLevel_->GetWidth() / 2, hpTop.y_ - classLevel_->GetHeight());
                SetUIElementSizePos(classLevel_, IntVector2::ZERO, ihpPos);
            }
        }
    }

    if (nameWindow_)
        nameWindow_->SetVisible((highlight || hovered_ || playerSelected_) && inRange);
    if (hpBar_ && !undestroyable_)
    {
        hpBar_->SetValues(stats_.maxHealth, stats_.health);
        hpBar_->SetVisible(((hovered_ && objectType_ != ObjectType::Self) || playerSelected_ || highlight) && inRange && !IsDead());
    }
    else if (classLevel_)
    {
        classLevel_->SetVisible(((hovered_ && objectType_ != ObjectType::Self) || playerSelected_ || highlight) && inRange);
    }
    if (IsSpeechBubbleVisible())
    {
        IntVector2 isbPos(screenPos.x_ - (speechBubbleWindow_->GetWidth() / 3), hpTop.y_ - 40);
        SetUIElementSizePos(speechBubbleWindow_, IntVector2::ZERO, isbPos);
        speechBubbleVisible_ += timeStep;
        if (speechBubbleVisible_ > 3.0f)
            HideSpeechBubble();
    }
}

void Actor::MoveTo(int64_t, const Vector3& newPos)
{
    SetMoveToPos(newPos);
}

void Actor::ForcePosition(int64_t, const Vector3& newPos)
{
    SetMoveToPos(newPos);
}

void Actor::SetYRotation(int64_t, float rad, bool)
{
    auto* smoothTransform = node_->GetComponent<SmoothedTransform>();
    const Quaternion rotateTo = { NormalizedAngle(RadToDeg(rad)), Vector3::UP };
    smoothTransform->SetTargetRotation(rotateTo);
}

void Actor::RemoveFromScene()
{
    RemoveActorUI();
}

void Actor::AddActorUI()
{
    if (!selectable_)
        return;
    if (name_.Empty())
        return;
    ASSERT(name_.Compare("Unknown") != 0);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();

    speechBubbleWindow_ = uiRoot->CreateChild<Window>();
    speechBubbleWindow_->SetLayoutMode(LM_HORIZONTAL);
    speechBubbleWindow_->SetLayoutBorder(IntRect(6, 3, 6, 3));
    speechBubbleWindow_->SetTexture(tex);
    speechBubbleWindow_->SetImageRect(IntRect(48, 0, 64, 16));
    speechBubbleWindow_->SetBorder(IntRect(4, 4, 4, 4));
    speechBubbleWindow_->SetImageBorder(IntRect(0, 0, 0, 0));
    speechBubbleWindow_->SetVisible(false);
    speechBubbleWindow_->SetOpacity(0.8f);
    speechBubbleText_ = speechBubbleWindow_->CreateChild<Text>("SpeechBubbleText");
//    speechBubbleText_->SetWordwrap(true);
    speechBubbleText_->SetMaxWidth(400);
    speechBubbleText_->SetAlignment(HA_LEFT, VA_TOP);
    speechBubbleText_->SetStyle("ActorNameText");
    speechBubbleText_->SetVisible(true);

    nameWindow_ = uiRoot->CreateChild<Window>();
    nameWindow_->SetVar("Actor", this);
    nameWindow_->SetLayoutMode(LM_VERTICAL);
    nameWindow_->SetLayoutBorder(IntRect(8, 4, 8, 4));
    nameWindow_->SetTexture(tex);
    nameWindow_->SetImageRect(IntRect(48, 0, 64, 16));
    nameWindow_->SetBorder(IntRect(4, 4, 4, 4));
    nameWindow_->SetImageBorder(IntRect(0, 0, 0, 0));
    nameWindow_->SetVisible(false);
    nameWindow_->SetOpacity(0.5f);

    nameLabel_ = nameWindow_->CreateChild<Text>();
    nameLabel_->SetAlignment(HA_LEFT, VA_CENTER);
    nameLabel_->SetStyle("ActorNameText");
    if (count_ > 1)
        nameLabel_->SetText(String(count_) + " " + name_);
    else
        nameLabel_->SetText(name_);
    nameLabel_->SetVisible(true);
    SubscribeToEvent(nameWindow_, E_CLICK, URHO3D_HANDLER(Actor, HandleNameClicked));
    SubscribeToEvent(nameWindow_, E_DOUBLECLICK, URHO3D_HANDLER(Actor, HandleNameDoubleClicked));

    if (HasHealthBar())
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        if (!AB::Entities::IsOutpost(lm->GetMapType()))
        {
            // No HP bar in outposts
            hpBar_ = uiRoot->CreateChild<ValueBar>();
            hpBar_->SetRange(100.0f);
            hpBar_->SetStyle("HealthBarGreen");
            hpBar_->SetSize(100, 20);
            hpBar_->SetValue(50.0f);
            hpBar_->SetVisible(false);
        }
        else
        {
            if (IsPlayingCharacter())
            {
                classLevel_ = uiRoot->CreateChild<Text>();
                classLevel_->SetStyleAuto();
                classLevel_->SetVisible(false);
                classLevel_->SetText(GetClassLevel());
                classLevel_->SetFontSize(8);
            }
        }
    }
}

void Actor::HandleObjectSecProfessionChange(StringHash, VariantMap& eventData)
{
    {
        using namespace Events::SetSecProfession;

        uint32_t objectId = eventData[P_OBJECTID].GetUInt();
        if (objectId != gameId_)
            return;

        uint32_t profIndex = eventData[P_PROFINDEX].GetUInt();
        if (profession2_->index == profIndex)
            return;

        auto* sm = GetSubsystem<SkillManager>();
        profession2_ = sm->GetProfessionByIndex(profIndex);
        ResetSecondProfAttributes();

        if (classLevel_)
            classLevel_->SetText(GetClassLevel());
    }
    {
        using namespace Events::ActorSkillsChanged;
        VariantMap& eData = GetEventDataMap();
        eData[P_OBJECTID] = gameId_;
        eData[P_UPDATEALL] = true;
        SendEvent(Events::E_ACTOR_SKILLS_CHANGED, eData);
    }
}

void Actor::HandleLoadSkillTemplate(StringHash, VariantMap& eventData)
{
    {
        using namespace Events::LoadSkillTemplate;

        uint32_t objectId = eventData[P_OBJECTID].GetUInt();
        if (objectId != gameId_)
            return;

        const String& templ = eventData[P_TEMPLATE].GetString();
        LoadSkillTemplate(std::string(templ.CString()));
    }

    {
        using namespace Events::ActorSkillsChanged;
        VariantMap& eData = GetEventDataMap();
        eData[P_OBJECTID] = gameId_;
        eData[P_UPDATEALL] = true;
        SendEvent(Events::E_ACTOR_SKILLS_CHANGED, eData);
    }
}

void Actor::RemoveActorUI()
{
    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();
    if (nameWindow_)
    {
        UnsubscribeFromEvent(nameWindow_, E_CLICK);
        uiRoot->RemoveChild(nameWindow_);
        nameWindow_.Reset();
        nameLabel_.Reset();
    }
    if (hpBar_)
    {
        uiRoot->RemoveChild(hpBar_);
        hpBar_.Reset();
    }
    if (speechBubbleWindow_)
    {
        uiRoot->RemoveChild(speechBubbleWindow_);
        speechBubbleWindow_.Reset();
    }
    if (classLevel_)
    {
        uiRoot->RemoveChild(classLevel_);
        classLevel_.Reset();
    }
}

String Actor::GetClassSubdir(AB::Entities::ModelClass cls)
{
    switch (cls)
    {
    case AB::Entities::ModelClass::WarriorFemale:
        return "W/F/";
    case AB::Entities::ModelClass::WarriorMale:
        return "W/M/";
    case AB::Entities::ModelClass::ElementaristFemale:
        return "E/F/";
    case AB::Entities::ModelClass::ElementaristMale:
        return "E/M/";
    case AB::Entities::ModelClass::MesmerFemale:
        return "Me/F/";
    case AB::Entities::ModelClass::MesmerMale:
        return "Me/M/";
    case AB::Entities::ModelClass::NecromancerFemale:
        return "N/F/";
    case AB::Entities::ModelClass::NecromancerMale:
        return "N/M/";
    case AB::Entities::ModelClass::PriestFemale:
        return "Mo/F/";
    case AB::Entities::ModelClass::PriestMale:
        return "Mo/M/";
    case AB::Entities::ModelClass::RangerFemale:
        return "R/F/";
    case AB::Entities::ModelClass::RangerMale:
        return "R/M/";
    default:
        return String::EMPTY;
    }
}

String Actor::GetAnimation(AB::Entities::ModelClass cls, const StringHash& hash)
{
    String result = "Animations/";
    if (cls == AB::Entities::ModelClass::AccountChest)
    {
        if (hash == ANIM_CHEST_OPENING)
            return result + "Chest/Opening.ani";
        if (hash == ANIM_CHEST_CLOSING)
            return result + "Chest/Closing.ani";
    }

    result += GetClassSubdir(cls);
    if (hash == ANIM_IDLE)
        result += "Idle.ani";
    else if (hash == ANIM_DYING)
        result += "Dying.ani";
    else if (hash == ANIM_CRY)
        result += "Crying.ani";
    else if (hash == ANIM_RUN)
        result += "Running.ani";
    else if (hash == ANIM_WALK)
        result += "Walking.ani";
    else if (hash == ANIM_SIT)
        result += "Sitting.ani";
    else if (hash == ANIM_CASTING)
        result += "Casting.ani";
    else if (hash == ANIM_TAUNTING)
        result += "Taunting.ani";
    else if (hash == ANIM_PONDER)
        result += "Ponder.ani";
    else if (hash == ANIM_WAVE)
        result += "Wave.ani";
    else if (hash == ANIM_LAUGH)
        result += "Laugh.ani";
    else if (hash == ANIM_ATTACK)
        result += "Attack.ani";
    else
        return "";
    return result;
}

String Actor::GetAnimation(const StringHash& hash)
{
    return GetAnimation(modelClass_, hash);
}

String Actor::GetSoundEffect(const StringHash& hash)
{
    String result;
    result = "Sounds/FX/";
    if (hash == SOUND_SKILLFAILURE)
    {
        return result + "SkillFailure.wav";
    }
    else if (hash == SOUND_FOOTSTEPS)
    {
        return result + "footsteps_run1.wav";
    }

    if (!profession_ || profession_->abbr.empty())
        return String::EMPTY;

    result += "Characters/" + GetClassSubdir(modelClass_);

    if (hash == SOUND_DIE)
        result += "Dying.wav";
    else
        return "";
    return result;
}

void Actor::HandleNameClicked(StringHash, VariantMap&)
{
    if (nameLabel_->IsVisible())
    {
        VariantMap& eData = GetEventDataMap();
        using namespace Events::ActorNameClicked;
        eData[P_SOURCEID] = gameId_;
        SendEvent(Events::E_ACTORNAMECLICKED, eData);
    }
}

void Actor::HandleNameDoubleClicked(StringHash, VariantMap&)
{
    if (nameLabel_->IsVisible())
    {
        VariantMap& eData = GetEventDataMap();
        using namespace Events::ActorNameDoubleClicked;
        eData[P_SOURCEID] = gameId_;
        SendEvent(Events::E_ACTORNAMEDOUBLECLICKED, eData);
    }
}

void Actor::HandleAnimationFinished(StringHash, VariantMap& eventData)
{
    if (objectType_ == ObjectType::Self)
    {
        using namespace AnimationFinished;
        bool looped = eventData[P_LOOPED].GetBool();
        if (!looped)
        {
            // Reset to idle when some emote animations ended
            if (creatureState_ > AB::GameProtocol::CreatureState::__EmoteStart &&
                creatureState_ < AB::GameProtocol::CreatureState::__EmoteEnd)
            {
                FwClient* client = GetSubsystem<FwClient>();
                client->SetPlayerState(AB::GameProtocol::CreatureState::Idle);
            }
        }
    }
}

void Actor::ShowSpeechBubble(const String& text)
{
    if (text.Empty())
        return;
    if (GetDistanceToPlayer() > Game::RANGE_SELECT)
        return;

    speechBubbleWindow_->SetSize(0, 0);
    speechBubbleText_->SetSize(0, 0);
    String message(text);
    speechBubbleVisible_ = 0;
    if (message.Length() > 53)
        message = message.Substring(0, 50) + "...";
    speechBubbleText_->SetText(message);

    speechBubbleWindow_->UpdateLayout();

    speechBubbleWindow_->SetWidth(speechBubbleText_->GetWidth());
    speechBubbleWindow_->SetVisible(true);
    speechBubbleWindow_->BringToFront();
}

void Actor::HideSpeechBubble()
{
    speechBubbleWindow_->SetVisible(false);
    speechBubbleText_->SetText(String::EMPTY);
}

void Actor::HandleChatMessage(StringHash, VariantMap& eventData)
{
    using namespace Events::ChatMessage;
    uint32_t senderId = static_cast<uint32_t>(eventData[P_SENDERID].GetInt());
    if (senderId != gameId_)
        return;

    // Show what we say in a bubble for certain channels
    AB::GameProtocol::ChatChannel channel =
        static_cast<AB::GameProtocol::ChatChannel>(eventData[P_MESSAGETYPE].GetInt());

    if (channel == AB::GameProtocol::ChatChannel::General || channel == AB::GameProtocol::ChatChannel::Party)
    {
        const String& message = eventData[P_DATA].GetString();
        auto* chatFilter = GetSubsystem<ChatFilter>();
        if (chatFilter->Matches(message))
            return;
        ShowSpeechBubble(message);
    }
}

void Actor::HandleSkillUse(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectUseSkill;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != gameId_)
        return;
    int skillIndex = eventData[P_SKILLINDEX].GetInt();
    if (skillIndex < 1 || skillIndex > Game::PLAYER_MAX_SKILLS)
        return;
    uint32_t skill = skills_[static_cast<size_t>(skillIndex - 1)];
    const AB::Entities::Skill* pSkill = GetSubsystem<SkillManager>()->GetSkillByIndex(skill);
    if (pSkill)
    {
        if (!pSkill->soundEffect.empty())
        {
            const String s(pSkill->soundEffect.c_str(), static_cast<unsigned>(pSkill->soundEffect.length()));
            const String name("Skill_" + String(pSkill->index));
            PlaySoundEffect(s, name);
        }
    }
}

void Actor::HandleEndSkillUse(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectEndUseSkill;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != gameId_)
        return;
/*    int skillIndex = eventData[P_SKILLINDEX].GetInt();
    if (skillIndex < 1 || skillIndex > PLAYER_MAX_SKILLS)
        return;
    uint32_t skill = skills_[skillIndex - 1];
    const String name("Skill_" + String(skill));
    Node* nd = node_->GetChild(name);
    if (nd)
    {
        nd->Remove();
    }*/
}

void Actor::HandleEffectAdded(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectEffectAdded;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != gameId_)
        return;
    uint32_t effectIndex = eventData[P_EFFECTINDEX].GetUInt();
    SkillManager* sm = GetSubsystem<SkillManager>();
    const AB::Entities::Effect* pEffect = sm->GetEffectByIndex(effectIndex);
    if (pEffect)
    {
        if (!pEffect->soundEffect.empty())
        {
            const String s(pEffect->soundEffect.c_str(), static_cast<unsigned>(pEffect->soundEffect.length()));
            const String name("Effect_" + String(pEffect->index));
            PlaySoundEffect(s, name);
        }
    }
}

void Actor::HandleItemDropped(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectItemDropped;
    uint32_t itemId = eventData[P_ITEMID].GetUInt();
    if (itemId == gameId_)
    {
        count_ = eventData[P_COUNT].GetUInt();
        value_ = eventData[P_VALUE].GetUInt();
        if (count_ > 1)
            nameLabel_->SetText(String(count_) + " " + name_);
        else
            nameLabel_->SetText(name_);
        ItemStats stats;
        LoadStatsFromString(stats, eventData[P_STATS].GetString());
        if (stats.Size() != 0)
        {
            auto* statsElem = nameWindow_->CreateChild<ItemStatsUIElement>();
            statsElem->SetStats(stats);
        }
        dropTarget_ = eventData[P_TARGETID].GetUInt();
        if (dropTarget_ != 0)
        {
            auto* lm = GetSubsystem<LevelManager>();
            auto* target = lm->GetObject(dropTarget_);
            if (target && Is<Actor>(target))
            {
                auto* targetActor = To<Actor>(target);
                Text* targetName = nameWindow_->GetChildStaticCast<Text>("DropTarget", true);
                if (!targetName)
                {
                    targetName = nameWindow_->CreateChild<Text>("DropTarget");
                    targetName->SetStyle("DropTargetNameText");
                }
                targetName->SetText("For " + targetActor->name_);
            }
        }
    }
}

void Actor::SetSelectedObject(SharedPtr<GameObject> object)
{
    if (selectedObject_ == object)
        return;

    if (objectType_ == ObjectType::Self && selectedObject_)
    {
        selectedObject_->playerSelected_ = false;
    }
    selectedObject_ = object;
    if (objectType_ == ObjectType::Self && selectedObject_)
    {
        selectedObject_->playerSelected_ = true;
    }
}

void Actor::UpdateMoveSpeed()
{
    if (!animController_)
        return;

    switch (creatureState_)
    {
    case AB::GameProtocol::CreatureState::Moving:
    {
        if (speedFactor_ > WALK_SPEED_THRESHOLD && currentAnimation_ != ANIM_RUN)
        {
            PlayAnimation(ANIM_RUN, true, 0.0f, RUN_ANIM_SPEED(speedFactor_));
            return;
        }
        else if (speedFactor_ <= WALK_SPEED_THRESHOLD && currentAnimation_ != ANIM_WALK)
        {
            // speed / 2 -> walk animation -> playing at normal speed = speed * 2
            PlayAnimation(ANIM_WALK, true, 0.0f, WALK_ANIM_SPEED(speedFactor_));
            return;
        }
        break;
    }
    default:
        break;
    }

    // Change speed of currently running animations
    const String& aniRun = animations_[ANIM_RUN];
    if (!aniRun.Empty())
        animController_->SetSpeed(aniRun, RUN_ANIM_SPEED(speedFactor_));
    const String& aniWalk = animations_[ANIM_WALK];
    if (!aniWalk.Empty())
        // speed / 2 -> walk animation -> playing at normal speed = speed * 2
        animController_->SetSpeed(aniWalk, WALK_ANIM_SPEED(speedFactor_));
}

void Actor::PlayAnimation(StringHash animation, bool looped /* = true */, float fadeTime /* = 0.2f */, float speed /* = 1.0f */)
{
    if (!animController_)
        return;

    const String& ani = animations_[animation];
    if (!ani.Empty())
    {
        currentAnimation_ = animation;
        animController_->PlayExclusive(ani, 0, looped, fadeTime);
        // Play adds the animation then we can set the speed of it
        animController_->SetSpeed(ani, speed);
    }
    else
    {
        animController_->StopAll();
        currentAnimation_ = StringHash::ZERO;
    }
}

void Actor::PlayObjectAnimation(bool looped, float fadeTime, float speed)
{
    // Play the animation referenced in the Object node
    const String& fileName = node_->GetVar("AnimationFile").GetString();
    AnimationController* animCtrl = node_->GetComponent<AnimationController>();
    if (!fileName.Empty() && animCtrl)
    {
        animController_->PlayExclusive(fileName, 0, looped, fadeTime);
        animController_->SetSpeed(fileName, speed);
    }
    ParticleEmitter* pe = node_->GetComponent<ParticleEmitter>(true);
    if (pe)
    {
        pe->SetEmitting(true);
    }
}

void Actor::PlayIdleAnimation(float fadeTime)
{
    PlayAnimation(ANIM_IDLE, true, fadeTime);
    ParticleEmitter* pe = node_->GetComponent<ParticleEmitter>(true);
    if (pe)
        pe->SetEmitting(false);
}

void Actor::PlayStateAnimation(float fadeTime)
{
    switch (creatureState_)
    {
    case AB::GameProtocol::CreatureState::Idle:
        PlayIdleAnimation(fadeTime);
        break;
    case AB::GameProtocol::CreatureState::Moving:
    {
        if (speedFactor_ > WALK_SPEED_THRESHOLD)
            PlayAnimation(ANIM_RUN, true, fadeTime, RUN_ANIM_SPEED(speedFactor_));
        else
            // speed / 2 -> walk animation -> playing at normal speed = speed * 2
            PlayAnimation(ANIM_WALK, true, fadeTime, WALK_ANIM_SPEED(speedFactor_));
        break;
    }
    case AB::GameProtocol::CreatureState::UsingSkill:
        // Loop for extremely long spells
        PlayAnimation(ANIM_CASTING, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::Attacking:
        PlayAnimation(ANIM_ATTACK, true, fadeTime, attackSpeed_);
        break;
    case AB::GameProtocol::CreatureState::Emote:
        break;
    case AB::GameProtocol::CreatureState::EmoteSit:
        PlayAnimation(ANIM_SIT, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::EmoteCry:
        PlayAnimation(ANIM_CRY, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::EmoteTaunt:
        PlayAnimation(ANIM_TAUNTING, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::EmotePonder:
        PlayAnimation(ANIM_PONDER, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::EmoteWave:
        PlayAnimation(ANIM_WAVE, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::EmoteLaugh:
        PlayAnimation(ANIM_LAUGH, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::Dead:
        PlayAnimation(ANIM_DYING, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureState::ChestOpen:
        PlayAnimation(ANIM_CHEST_OPENING, false, 0.0f);
        break;
    case AB::GameProtocol::CreatureState::ChestClosed:
        PlayAnimation(ANIM_CHEST_CLOSING, false, 0.0f);
        break;
    case AB::GameProtocol::CreatureState::Triggered:
        // E.g. Traps have their animation referenced in the object node
        PlayObjectAnimation();
        break;
    default:
        break;
    }
}

void Actor::ChangeResource(AB::GameProtocol::ResourceType resType, int32_t value)
{
    switch (resType)
    {
    case AB::GameProtocol::ResourceType::Health:
        stats_.health = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceType::Energy:
        stats_.energy = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceType::Adrenaline:
        stats_.adrenaline = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceType::Overcast:
        stats_.overcast = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceType::HealthRegen:
        stats_.healthRegen = value;
        break;
    case AB::GameProtocol::ResourceType::EnergyRegen:
        stats_.energyRegen = value;
        break;
    case AB::GameProtocol::ResourceType::MaxHealth:
        stats_.maxHealth = static_cast<unsigned>(value);
        break;
    case AB::GameProtocol::ResourceType::MaxEnergy:
        stats_.maxEnergy = static_cast<unsigned>(value);
        break;
    case AB::GameProtocol::ResourceType::Morale:
        stats_.morale = value;
        break;
    }
}

void Actor::SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState)
{
    AB::GameProtocol::CreatureState prevState = creatureState_;
    GameObject::SetCreatureState(time, newState);

    float fadeTime = 0.2f;
    switch (creatureState_)
    {
    case AB::GameProtocol::CreatureState::Idle:
//        if (prevState != AB::GameProtocol::CreatureStateEmoteSit)
//            fadeTime = 0.0f;
//        else
        if (prevState == AB::GameProtocol::CreatureState::EmoteSit)
            fadeTime = 0.5f;
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureState::Moving:
    {
        PlaySoundEffect(SOUND_FOOTSTEPS, true);
        break;
    }
    case AB::GameProtocol::CreatureState::UsingSkill:
        break;
    case AB::GameProtocol::CreatureState::Attacking:
        break;
    case AB::GameProtocol::CreatureState::Emote:
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureState::EmoteSit:
        fadeTime = 0.5f;
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureState::EmoteCry:
        PlaySoundEffect(SOUND_NONE, true);
        fadeTime = 0.5f;
        break;
    case AB::GameProtocol::CreatureState::Dead:
    {
        PlaySoundEffect(SOUND_NONE, true);
        PlaySoundEffect(SOUND_DIE);
        break;
    }
    default:
        break;
    }
    PlayStateAnimation(fadeTime);
}

void Actor::SetSpeedFactor(int64_t time, float value)
{
    GameObject::SetSpeedFactor(time, value);
    UpdateMoveSpeed();
}

void Actor::Unserialize(sa::PropReadStream& data)
{
    using namespace AB::GameProtocol;

    uint32_t validFields;
    if (!data.Read<uint32_t>(validFields))
        return;

    if (validFields & ObjectSpawnDataFieldName)
    {
        std::string str;
        if (data.ReadString(str))
            name_ = String(str.data(), static_cast<unsigned>(str.length()));
    }

    if (validFields & ObjectSpawnDataFieldLevel)
        data.Read(level_);
    if (validFields & ObjectSpawnDataFieldPvpCharacter)
    {
        uint8_t isPvp;
        if (data.Read(isPvp))
            pvpCharacter_ = isPvp != 0;
    }

    if (validFields & ObjectSpawnDataFieldSex)
    {
        uint8_t s;
        if (data.Read(s))
            sex_ = static_cast<AB::Entities::CharacterSex>(s);
    }

    if (validFields & ObjectSpawnDataFieldProf)
    {
        SkillManager* sm = GetSubsystem<SkillManager>();
        {
            uint32_t p;
            data.Read(p);
            profession_ = sm->GetProfessionByIndex(p);
        }
        {
            uint32_t p;
            data.Read(p);
            profession2_ = sm->GetProfessionByIndex(p);
        }
    }
    if (validFields & ObjectSpawnDataFieldModelIndex)
        data.Read(itemIndex_);

    if (validFields & ObjectSpawnDataFieldSkills)
    {
        std::string skills;
        data.ReadString(skills);
        LoadSkillTemplate(skills);
    }
}

void Actor::PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop /* = false */)
{
    if (!soundSource)
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    if (sounds_.Contains(type) && cache->Exists(sounds_[type]))
    {
        Sound* sound = cache->GetResource<Sound>(sounds_[type]);
        sound->SetLooped(loop);
        soundSource->Play(sound);
    }
    else if (type == SOUND_NONE && soundSource->IsPlaying())
    {
        soundSource->Stop();
    }
}

void Actor::PlaySoundEffect(const StringHash& type, bool loop)
{
    if (loop)
        PlaySoundEffect(soundSource_, type, loop);
    else
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        if (sounds_.Contains(type) && cache->Exists(sounds_[type]))
        {
            PlaySoundEffect(sounds_[type]);
        }
    }
}

void Actor::PlaySoundEffect(const String& fileName, const String& name /* = String::EMPTY */)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Sound* sound = cache->GetResource<Sound>(fileName);
    if (!sound)
        return;

    Node* nd = node_->CreateChild(name);
    auto* soundSource = nd->CreateComponent<SoundSource3D>();
    soundSource->SetSoundType(SOUND_EFFECT);
    soundSource->SetAutoRemoveMode(REMOVE_NODE);
    soundSource->SetNearDistance(2.0f);
    soundSource->SetFarDistance(15.0f);
    soundSource->Play(sound);
}

bool Actor::LoadSkillTemplate(const std::string& templ)
{
    AB::Entities::Profession p1;
    AB::Entities::Profession p2;
    Game::Attributes attribs;
    Game::SkillIndices skills;
    if (!IO::SkillTemplateDecode(templ, p1, p2, attribs, skills))
        return false;
    attributes_ = attribs;
    skills_ = skills;
    profession2_ = GetSubsystem<SkillManager>()->GetProfessionByIndex(p2.index);
    return true;
}

std::string Actor::SaveSkillTemplate()
{
    AB::Entities::Profession prof1;
    if (profession_)
        prof1 = *profession_;
    AB::Entities::Profession prof2;
    if (profession2_)
        prof2 = *profession2_;
    std::string result = IO::SkillTemplateEncode(prof1, prof2, attributes_, skills_);
    return result;
}

void Actor::OnSkillError(AB::GameProtocol::SkillError)
{
    PlaySoundEffect(SOUND_SKILLFAILURE);
}

String Actor::GetClasses() const
{
    if (!profession_ || profession_->abbr.empty() || profession_->abbr.compare("NA") == 0)
        return String::EMPTY;
    String result = String(profession_->abbr.c_str());
    if (profession2_ && !profession2_->abbr.empty() && profession2_->abbr.compare("NA") != 0)
        result += "/" + String(profession2_->abbr.c_str());
    return result;
}

String Actor::GetClassLevel() const
{
    String result = GetClasses();
    if (result.Empty() && level_ == 0)
        return String::EMPTY;
    if (result.Empty())
        result = "Lvl";
    result += String(level_);
    return result;
}

String Actor::GetClassLevelName() const
{
    String result = GetClassLevel();
    if (result.Empty())
        return name_;
    return result + " " + name_;
}

void Actor::HandlePartyAdded(StringHash, VariantMap& eventData)
{
    using namespace Events::PartyAdded;
    uint32_t targetId = eventData[P_PLAYERID].GetUInt(); // Actor
    if (targetId == gameId_)
    {
        groupId_ = eventData[P_PARTYID].GetUInt();
    }
}

void Actor::HandlePartyRemoved(StringHash, VariantMap& eventData)
{
    using namespace Events::PartyRemoved;
    uint32_t targetId = eventData[P_TARGETID].GetUInt(); // Actor
    if (targetId == gameId_)
    {
        groupId_ = 0;
    }
}

uint32_t Actor::GetAttributeRank(Game::Attribute index) const
{
    return Game::GetAttribRank(attributes_, index);
}

void Actor::SetAttributeRank(Game::Attribute index, uint32_t value)
{
    Game::SetAttribRank(attributes_, index, value);
}

void Actor::ResetSecondProfAttributes()
{
    Game::InitProf2Attribs(attributes_, *profession_, profession2_);
}

int Actor::GetAttributePoints() const
{
    return static_cast<int>(Game::GetAttribPoints(level_));
}

int Actor::GetUsedAttributePoints() const
{
    return GetUsedAttribPoints(attributes_);
}

int Actor::GetAvailableAttributePoints() const
{
    return GetAttributePoints() - GetUsedAttributePoints();
}

bool Actor::CanIncreaseAttributeRank(Game::Attribute index) const
{
    uint32_t currRank = GetAttributeRank(index);
    if (currRank >= Game::MAX_PLAYER_ATTRIBUTE_RANK)
        return false;

    int cost = Game::CalcAttributeCost(static_cast<int>(currRank) + 1);
    int used = GetUsedAttribPoints(attributes_, static_cast<int>(index));
    int total = GetAttributePoints();

    return cost <= total - used;
}

void Actor::HandleSetAttribValue(StringHash, VariantMap& eventData)
{
    using namespace Events::SetAttributeValue;
    if (eventData[P_OBJECTID].GetUInt() != gameId_)
        return;

    uint32_t attribIndex = eventData[P_ATTRIBINDEX].GetUInt();
    int value = eventData[P_VALUE].GetInt();
    SetAttributeRank(static_cast<Game::Attribute>(attribIndex), static_cast<unsigned>(value));
}

void Actor::HandleSetSkill(StringHash, VariantMap& eventData)
{
    bool update = false;
    {
        using namespace Events::SetSkill;
        if (eventData[P_OBJECTID].GetUInt() != gameId_)
            return;

        uint32_t skillIndex = eventData[P_SKILLINDEX].GetUInt();
        unsigned skillPos = eventData[P_SKILLPOS].GetUInt();
        if (skillPos < Game::PLAYER_MAX_SKILLS)
        {
            skills_[skillPos] = skillIndex;
            update = true;
        }
    }
    if (update)
    {
        using namespace Events::ActorSkillsChanged;
        VariantMap& eData = GetEventDataMap();
        eData[P_OBJECTID] = gameId_;
        eData[P_UPDATEALL] = false;
        SendEvent(Events::E_ACTOR_SKILLS_CHANGED, eData);
    }
}

void Actor::HandleGroupMaskChanged(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectGroupMaskChanged;
    if (eventData[P_OBJECTID].GetUInt() != gameId_)
        return;

    groupMask_ = eventData[P_GROUPMASK].GetUInt();
}

void Actor::HandleSetAttackSpeed(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectSetAttackSpeed;
    if (eventData[P_OBJECTID].GetUInt() != gameId_)
        return;

    attackSpeed_ = eventData[P_SPEED].GetFloat();
    const String& ani = animations_[ANIM_ATTACK];
    if (!ani.Empty())
        animController_->SetSpeed(ani, attackSpeed_);
}

void Actor::HandleDropTargetChanged(StringHash, VariantMap& eventData)
{
    using namespace Events::DropTargetChanged;
    if (eventData[P_OBJECTID].GetUInt() != gameId_)
        return;
    dropTarget_ = eventData[P_TARGETID].GetUInt();
    Text* targetName = nameWindow_->GetChildStaticCast<Text>("DropTarget", true);
    if (targetName)
        targetName->Remove();
}
