// Most of it taken from the Character Demo

#include "stdafx.h"
#include "Actor.h"
#include "Definitions.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#include "MathUtils.h"
#include "TimeUtils.h"
#include <AB/ProtocolCodes.h>
#include "FwClient.h"
#include "ItemsCache.h"
#include "Shortcuts.h"
#include <AB/Entities/Skill.h>
#include "HealthBar.h"
#include "SkillManager.h"

#include <Urho3D/DebugNew.h>

Actor::Actor(Context* context) :
    GameObject(context),
    pickable_(false),
    animatedModel_(nullptr),
    type_(ModelType::Static),
    animController_(nullptr),
    model_(nullptr),
    selectedObject_(nullptr),
    nameLabel_(nullptr),
    name_(""),
    profession_(nullptr),
    profession2_(nullptr),
    sex_(AB::Entities::CharacterSexUnknown),
    level_(0),
    skills_{},
    speechBubbleVisible_(false)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_UPDATE);
    SubscribeToEvent(AbEvents::E_CHATMESSAGE, URHO3D_HANDLER(Actor, HandleChatMessage));
    SubscribeToEvent(AbEvents::E_OBJECTUSESKILL, URHO3D_HANDLER(Actor, HandleSkillUse));
    SubscribeToEvent(AbEvents::E_OBJECTENDUSESKILL, URHO3D_HANDLER(Actor, HandleEndSkillUse));
    SubscribeToEvent(AbEvents::E_OBJECTEFFECTADDED, URHO3D_HANDLER(Actor, HandleEffectAdded));
    SubscribeToEvent(AbEvents::E_PARTYADDED, URHO3D_HANDLER(Actor, HandlePartyAdded));
    SubscribeToEvent(AbEvents::E_PARTYREMOVED, URHO3D_HANDLER(Actor, HandlePartyRemoved));
    SubscribeToEvent(AbEvents::E_OBJECTITEMDROPPED, URHO3D_HANDLER(Actor, HandleItemDropped));
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
    const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state,
    PropReadStream& data)
{
    Node* node = scene->CreateChild(0, LOCAL);
    Actor* result = node->CreateComponent<Actor>();
    result->id_ = id;

    result->Unserialize(data);
    result->Init(scene, position, rotation, state);
    result->PlayStateAnimation(0.0f);

    return result;
}

void Actor::Init(Scene*, const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state)
{
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
    sounds_[SOUND_SKILLFAILURE] = GetSoundEffect(SOUND_SKILLFAILURE);
    sounds_[SOUND_FOOTSTEPS] = GetSoundEffect(SOUND_FOOTSTEPS);
    sounds_[SOUND_DIE] = GetSoundEffect(SOUND_DIE);

    if (modelIndex_ != 0)
    {
        if (objectType_ != ObjectTypeAreaOfEffect)
            LoadModel(modelIndex_, position, rotation);
        else
        {
            LoadAreaOfEffect(modelIndex_, position, rotation);
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

bool Actor::LoadModel(uint32_t index, const Vector3& position, const Quaternion& rotation)
{
    ItemsCache* items = GetSubsystem<ItemsCache>();
    SharedPtr<Item> item = items->Get(index);
    if (!item)
    {
        URHO3D_LOGERRORF("Model Item not found: %d", index);
        return false;
    }
    XMLFile* object = item->GetModelResource<XMLFile>();
    if (!object)
    {
        URHO3D_LOGERRORF("Prefab file not found for %s: %s", item->name_.CString(), item->modelFile_.CString());
        return false;
    }

    XMLElement root = object->GetRoot();
    unsigned nodeId = root.GetUInt("id");
    SceneResolver resolver;
    Node* adjNode = node_->CreateChild(0, LOCAL);
    resolver.AddNode(nodeId, adjNode);
    adjNode->SetRotation(Quaternion(270, Vector3(0, 1, 0)));
    if (adjNode->LoadXML(root, resolver, true, true))
    {
        resolver.Resolve();
        node_->SetTransform(position, rotation);
        adjNode->ApplyAttributes();
        if (adjNode->GetComponent<AnimatedModel>(true))
        {
            AnimatedModel* animModel = adjNode->GetComponent<AnimatedModel>(true);
            type_ = Actor::Animated;
            animController_ = adjNode->CreateComponent<AnimationController>();
            model_ = animModel;
        }
        else
        {
            type_ = Actor::Static;
            model_ = adjNode->GetComponent<StaticModel>(true);
        }
        Node* soundSourceNode = node_->CreateChild("SoundSourceNode");
        soundSource_ = soundSourceNode->CreateComponent<SoundSource3D>();
        soundSource_->SetSoundType(SOUND_EFFECT);
        soundSource_->SetNearDistance(2.0f);
        soundSource_->SetFarDistance(15.0f);
    }
    else
    {
        URHO3D_LOGERRORF("Error instantiating prefab %s", item->modelFile_.CString());
        adjNode->Remove();
        return false;
    }
    return true;
}

bool Actor::LoadAreaOfEffect(uint32_t index, const Vector3& position, const Quaternion& rotation)
{
    // TODO
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
    XMLFile* xml = item->GetModelResource<XMLFile>();
    if (!xml)
    {
        URHO3D_LOGERRORF("Prefab file not found: %s", item->modelFile_.CString());
        return;
    }

    XMLElement root = xml->GetRoot();

/*    unsigned nodeId = root.GetUInt("id");
    SceneResolver resolver;
    Node* adjNode = node_->CreateChild(0, LOCAL);
    resolver.AddNode(nodeId, adjNode);
    adjNode->SetRotation(Quaternion(270, Vector3(0, 1, 0)));
    if (adjNode->LoadXML(root, resolver, true, true))
    {
        resolver.Resolve();
        adjNode->ApplyAttributes();
    }
    else
    {
        URHO3D_LOGERRORF("Error instantiating prefab %s", item->modelFile_);
        adjNode->Remove();
    }*/

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

void Actor::UpdateTransformation()
{
    Vector3 moveTo;
    if (creatureState_ == AB::GameProtocol::CreatureStateMoving)
    {
        FwClient* c = context_->GetSubsystem<FwClient>();
        double diff = (double)c->GetLastPing();
        float p[3];
        if (posExtrapolator_.ReadPosition(GetClientTime() - diff, p))
            moveTo = Vector3(p[0], p[1], p[2]);
        else
            moveTo = moveToPos_;
    }
    else
    {
        moveTo = moveToPos_;
    }

    const Vector3& cp = node_->GetPosition();
    if (moveTo != Vector3::ZERO && moveTo != cp)
    {
        // Try to make moves smoother...
        if ((cp - moveToPos_).LengthSquared() > 0.01f)
        {
            // Seems to be the best result
            Vector3 pos = cp.Lerp(moveTo, 0.4f);
            node_->SetPosition(pos);
        }
        else
            node_->SetPosition(moveTo);
    }

    const Quaternion& rot = node_->GetRotation();
    if (rotateTo_ != rot)
    {
        if (fabs(rotateTo_.YawAngle() - rot.YawAngle()) > 1.0f)
        {
            Quaternion r = rot.Slerp(rotateTo_, 0.4f);
            node_->SetRotation(r);
        }
        else
            node_->SetRotation(rotateTo_);
    }
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

void Actor::Update(float timeStep)
{
    UpdateTransformation();

    Shortcuts* sc = GetSubsystem<Shortcuts>();

    const Vector3& pos = node_->GetPosition();
    Vector3 headPos = GetHeadPos();
    headPos.y_ += 0.5f;
    IntVector2 screenPos = WorldToScreenPoint(pos);
    IntVector2 hpTop = WorldToScreenPoint(headPos);

    bool highlight = sc->Test(AbEvents::E_SC_HIGHLIGHTOBJECTS);
    if (hovered_ || playerSelected_ || highlight || speechBubbleWindow_->IsVisible())
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

            IntVector2 labelPos(screenPos.x_ - nameWindow_->GetWidth() / 2, screenPos.y_);
            nameWindow_->SetPosition(labelPos);

            if (hpBar_)
            {
                hpBar_->SetSize(static_cast<int>(130.f * sizeFac), static_cast<int>(18.f * sizeFac));
                IntVector2 ihpPos(screenPos.x_ - hpBar_->GetWidth() / 2, hpTop.y_ - hpBar_->GetHeight() - 5);
                hpBar_->SetPosition(ihpPos);
            }
            else if (classLevel_)
            {
                IntVector2 ihpPos(screenPos.x_ - classLevel_->GetWidth() / 2, hpTop.y_ - classLevel_->GetHeight());
                classLevel_->SetPosition(ihpPos);
            }
        }
    }

    nameWindow_->SetVisible(highlight || hovered_ || playerSelected_);
    if (hpBar_ && !undestroyable_)
    {
        hpBar_->SetValues(stats_.maxHealth, stats_.health);
        hpBar_->SetVisible((hovered_ && objectType_ != ObjectTypeSelf) || playerSelected_);
    }
    else if (classLevel_)
    {
        classLevel_->SetVisible((hovered_ && objectType_ != ObjectTypeSelf) || playerSelected_);
    }
    if (speechBubbleWindow_->IsVisible())
    {
        IntVector2 isbPos(screenPos.x_ - (speechBubbleWindow_->GetWidth() / 3), hpTop.y_ - 40);
        speechBubbleWindow_->SetPosition(isbPos);
        speechBubbleVisible_ += timeStep;
        if (speechBubbleVisible_ > 3.0f)
            HideSpeechBubble();
    }
}

void Actor::MoveTo(int64_t time, const Vector3& newPos)
{
    moveToPos_ = newPos;
    const float p[3] = { moveToPos_.x_, moveToPos_.y_, moveToPos_.z_ };
    posExtrapolator_.AddSample(GetServerTime(time), GetClientTime(), p);
}

void Actor::SetYRotation(int64_t, float rad, bool)
{
    float deg = RadToDeg(rad);
    rotateTo_.FromAngleAxis(deg, Vector3::UP);
}

void Actor::RemoveFromScene()
{
    RemoveActorUI();
}

void Actor::AddActorUI()
{
    if (name_.Empty())
        return;

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();

    speechBubbleWindow_ = uiRoot->CreateChild<Window>();
    speechBubbleWindow_->SetLayoutMode(LM_HORIZONTAL);
    speechBubbleWindow_->SetLayoutBorder(IntRect(6, 3, 6, 3));
    speechBubbleWindow_->SetPivot(0, 0);
    speechBubbleWindow_->SetTexture(tex);
    speechBubbleWindow_->SetImageRect(IntRect(48, 0, 64, 16));
    speechBubbleWindow_->SetBorder(IntRect(3, 3, 3, 3));
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
    nameWindow_->SetLayoutMode(LM_HORIZONTAL);
    nameWindow_->SetLayoutBorder(IntRect(8, 4, 8, 4));
    nameWindow_->SetPivot(0, 0);
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

    if (HasHealthBar())
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        if (lm->GetMapType() != AB::Entities::GameTypeOutpost)
        {
            // No HP bar in outposts
            hpBar_ = uiRoot->CreateChild<HealthBarPlain>();
            hpBar_->SetRange(100.0f);
            hpBar_->SetStyle("HealthBarGreen");
            hpBar_->SetSize(100, 20);
            hpBar_->SetValue(50.0f);
            hpBar_->SetVisible(false);
        }
        else
        {
            if (IsPlayer())
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

void Actor::RemoveActorUI()
{
    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();
    if (nameWindow_)
    {
        UnsubscribeFromEvent(nameWindow_, E_CLICK);
        uiRoot->RemoveChild(nameWindow_);
        nameWindow_ = SharedPtr<Window>();
        nameLabel_ = SharedPtr<Text>();
    }
    if (hpBar_)
    {
        uiRoot->RemoveChild(hpBar_);
        hpBar_ = SharedPtr<HealthBarPlain>();
    }
    if (speechBubbleWindow_)
    {
        uiRoot->RemoveChild(speechBubbleWindow_);
        speechBubbleWindow_ = SharedPtr<Window>();
    }
    if (classLevel_)
    {
        uiRoot->RemoveChild(classLevel_);
        classLevel_ = SharedPtr<Text>();
    }
}

String Actor::GetAnimation(const StringHash& hash)
{
    String result;
    result = "Animations/";
    result += String(profession_->abbr.c_str()) + "/";
    if (sex_ == AB::Entities::CharacterSexFemale)
        result += "F/";
    else
        result += "M/";

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

    result += "Characters/";
    result += String(profession_->abbr.c_str()) + "/";
    if (sex_ == AB::Entities::CharacterSexFemale)
        result += "F/";
    else
        result += "M/";

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
        using namespace AbEvents::ActorNameClicked;
        eData[P_SOURCEID] = id_;
        SendEvent(AbEvents::E_ACTORNAMECLICKED, eData);
    }
}

void Actor::HandleAnimationFinished(StringHash, VariantMap& eventData)
{
    if (objectType_ == ObjectTypeSelf)
    {
        using namespace AnimationFinished;
        bool looped = eventData[P_LOOPED].GetBool();
        if (!looped)
        {
            // Reset to idle when some emote animations ended
            if (creatureState_ > AB::GameProtocol::CreatureStateEmoteStart &&
                creatureState_ < AB::GameProtocol::CreatureStateEmoteEnd)
            {
                FwClient* client = GetSubsystem<FwClient>();
                client->SetPlayerState(AB::GameProtocol::CreatureStateIdle);
            }
        }
    }
}

void Actor::ShowSpeechBubble(const String& text)
{
    if (text.Empty())
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
    using namespace AbEvents::ChatMessage;
    uint32_t senderId = static_cast<uint32_t>(eventData[P_SENDERID].GetInt());
    if (senderId != id_)
        return;

    // Show what we say in a bubble for certain channels
    AB::GameProtocol::ChatMessageChannel channel =
        static_cast<AB::GameProtocol::ChatMessageChannel>(eventData[P_MESSAGETYPE].GetInt());

    if (channel == AB::GameProtocol::ChatChannelGeneral || channel == AB::GameProtocol::ChatChannelParty)
    {
        ShowSpeechBubble(eventData[P_DATA].GetString());
    }
}

void Actor::HandleSkillUse(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ObjectUseSkill;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != id_)
        return;
    int skillIndex = eventData[P_SKILLINDEX].GetInt();
    if (skillIndex < 1 || skillIndex > PLAYER_MAX_SKILLS)
        return;
    uint32_t skill = skills_[skillIndex - 1];
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
    using namespace AbEvents::ObjectEndUseSkill;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != id_)
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
    using namespace AbEvents::ObjectEffectAdded;
    uint32_t id = eventData[P_OBJECTID].GetUInt();
    if (id != id_)
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
    using namespace AbEvents::ObjectItemDropped;
    uint32_t itemId = eventData[P_ITEMID].GetUInt();
    if (itemId == id_)
    {
        count_ = eventData[P_COUNT].GetUInt();
        value_ = eventData[P_VALUE].GetUInt();
        if (count_ > 1)
            nameLabel_->SetText(String(count_) + " " + name_);
        else
            nameLabel_->SetText(name_);
    }
}

void Actor::SetSelectedObject(SharedPtr<GameObject> object)
{
    if (selectedObject_ == object)
        return;

    if (objectType_ == ObjectTypeSelf && selectedObject_)
    {
        selectedObject_->playerSelected_ = false;
    }
    selectedObject_ = object;
    if (objectType_ == ObjectTypeSelf && selectedObject_)
    {
        selectedObject_->playerSelected_ = true;
    }
}

void Actor::UpdateMoveSpeed()
{
    if (!animController_)
        return;

    // Change speed of currently running animations
    const String& aniRun = animations_[ANIM_RUN];
    if (!aniRun.Empty())
        animController_->SetSpeed(aniRun, speedFactor_);
    const String& aniWalk = animations_[ANIM_WALK];
    if (!aniWalk.Empty())
        // speed / 2 -> walk animation -> playing at normal speed = speed * 2
        animController_->SetSpeed(aniWalk, speedFactor_ * 2.0f);
}

void Actor::PlayAnimation(StringHash animation, bool looped /* = true */, float fadeTime /* = 0.2f */, float speed /* = 1.0f */)
{
    if (!animController_)
        return;

    const String& ani = animations_[animation];
    if (!ani.Empty())
    {
        animController_->PlayExclusive(ani, 0, looped, fadeTime);
        // Play adds the animation then we can set the speed of it
        animController_->SetSpeed(ani, speed);
    }
    else
        animController_->StopAll();
}

void Actor::PlayStateAnimation(float fadeTime)
{
    switch (creatureState_)
    {
    case AB::GameProtocol::CreatureStateIdle:
        PlayAnimation(ANIM_IDLE, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateMoving:
    {
        if (speedFactor_ > 0.5f)
            PlayAnimation(ANIM_RUN, true, fadeTime, speedFactor_);
        else
            // speed / 2 -> walk animation -> playing at normal speed = speed * 2
            PlayAnimation(ANIM_WALK, true, fadeTime, speedFactor_ * 2.0f);
        break;
    }
    case AB::GameProtocol::CreatureStateUsingSkill:
        // Loop for extremely long spells
        PlayAnimation(ANIM_CASTING, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateAttacking:
        PlayAnimation(ANIM_ATTACK, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmote:
        break;
    case AB::GameProtocol::CreatureStateEmoteSit:
        PlayAnimation(ANIM_SIT, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmoteCry:
        PlayAnimation(ANIM_CRY, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmoteTaunt:
        PlayAnimation(ANIM_TAUNTING, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmotePonder:
        PlayAnimation(ANIM_PONDER, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmoteWave:
        PlayAnimation(ANIM_WAVE, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmoteLaugh:
        PlayAnimation(ANIM_LAUGH, false, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateDead:
        PlayAnimation(ANIM_DYING, false, fadeTime);
        break;
    default:
        break;
    }
}

void Actor::ChangeResource(AB::GameProtocol::ResourceType resType, int32_t value)
{
    switch (resType)
    {
    case AB::GameProtocol::ResourceTypeHealth:
        stats_.health = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceTypeEnergy:
        stats_.energy = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceTypeAdrenaline:
        stats_.adrenaline = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceTypeOvercast:
        stats_.overcast = static_cast<unsigned>(Max(0, value));
        break;
    case AB::GameProtocol::ResourceTypeHealthRegen:
        stats_.healthRegen = value;
        break;
    case AB::GameProtocol::ResourceTypeEnergyRegen:
        stats_.energyRegen = value;
        break;
    case AB::GameProtocol::ResourceTypeMaxHealth:
        stats_.maxHealth = value;
        break;
    case AB::GameProtocol::ResourceTypeMaxEnergy:
        stats_.maxEnergy = value;
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
    case AB::GameProtocol::CreatureStateIdle:
//        if (prevState != AB::GameProtocol::CreatureStateEmoteSit)
//            fadeTime = 0.0f;
//        else
        if (prevState == AB::GameProtocol::CreatureStateEmoteSit)
            fadeTime = 0.5f;
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureStateMoving:
    {
        const float p[3] = { moveToPos_.x_, moveToPos_.y_, moveToPos_.z_ };
        posExtrapolator_.Reset(GetServerTime(time), GetClientTime(), p);
        PlaySoundEffect(SOUND_FOOTSTEPS, true);
        break;
    }
    case AB::GameProtocol::CreatureStateUsingSkill:
        break;
    case AB::GameProtocol::CreatureStateAttacking:
        break;
    case AB::GameProtocol::CreatureStateEmote:
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureStateEmoteSit:
        fadeTime = 0.5f;
        PlaySoundEffect(SOUND_NONE, true);
        break;
    case AB::GameProtocol::CreatureStateEmoteCry:
        PlaySoundEffect(SOUND_NONE, true);
        fadeTime = 0.5f;
        break;
    case AB::GameProtocol::CreatureStateDead:
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

void Actor::Unserialize(PropReadStream& data)
{
    GameObject::Unserialize(data);
    std::string str;
    if (data.ReadString(str))
        name_ = String(str.data(), static_cast<unsigned>(str.length()));
    uint8_t s;
    SkillManager* sm = GetSubsystem<SkillManager>();
    data.Read(level_);
    if (data.Read(s))
        sex_ = static_cast<AB::Entities::CharacterSex>(s);
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
    data.Read(modelIndex_);
    std::string skills;
    data.ReadString(skills);
    LoadSkillTemplate(skills);
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
    AB::Attributes attribs;
    AB::SkillIndices skills;
    if (!AB::TemplEncoder::Decode(templ, p1, p2, attribs, skills))
        return false;
    attributes_ = attribs;
    skills_ = skills;
    profession2_ = GetSubsystem<SkillManager>()->GetProfessionByIndex(p2.index);
    return true;
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
    using namespace AbEvents::PartyAdded;
    uint32_t targetId = eventData[P_PLAYERID].GetUInt(); // Actor
    if (targetId == id_)
    {
        groupId_ = eventData[P_PARTYID].GetUInt();
    }
}

void Actor::HandlePartyRemoved(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::PartyRemoved;
    uint32_t targetId = eventData[P_TARGETID].GetUInt(); // Actor
    if (targetId == id_)
    {
        groupId_ = 0;
    }
}
