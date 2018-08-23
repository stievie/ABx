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

#include <Urho3D/DebugNew.h>

Actor::Actor(Context* context) :
    GameObject(context),
    pickable_(false),
    animController_(nullptr),
    model_(nullptr),
    selectedObject_(nullptr),
    nameLabel_(nullptr),
    name_(""),
    profession_(nullptr),
    profession2_(nullptr),
    sex_(AB::Entities::CharacterSexUnknown)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE | USE_UPDATE);
}

Actor::~Actor()
{
}

void Actor::RegisterObject(Context* context)
{
    context->RegisterFactory<Actor>();
}

Actor* Actor::CreateActor(uint32_t id, Context* context, Scene* scene,
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

void Actor::Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
    AB::GameProtocol::CreatureState state)
{
    animations_[ANIM_RUN] = GetAnimation(ANIM_RUN);
    animations_[ANIM_IDLE] = GetAnimation(ANIM_IDLE);
    animations_[ANIM_SIT] = GetAnimation(ANIM_SIT);
    animations_[ANIM_DYING] = GetAnimation(ANIM_DYING);
    animations_[ANIM_CRY] = GetAnimation(ANIM_CRY);

    if (modelIndex_ != 0)
    {
        ItemsCache* items = GetSubsystem<ItemsCache>();
        SharedPtr<Item> item = items->Get(modelIndex_);
        if (!item)
        {
            URHO3D_LOGERRORF("Model Item not found: %d", modelIndex_);
            return;
        }
        XMLFile* object = item->GetModelResource<XMLFile>();
        if (!object)
        {
            URHO3D_LOGERRORF("Prefab file not found: %s", item->modelFile_);
            return;
        }

        XMLElement& root = object->GetRoot();
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
        }
        else
        {
            URHO3D_LOGERRORF("Error instantiating prefab %s", item->modelFile_);
            adjNode->Remove();
        }
    }
    else
        URHO3D_LOGERROR("Prefab file is empty");

    creatureState_ = state;
    if (model_)
    {
        model_->SetCastShadows(true);
        model_->SetOccludee(true);
        model_->SetOccluder(false);
    }
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
        URHO3D_LOGERRORF("Prefab file not found: %s", item->modelFile_);
        return;
    }

    XMLElement& root = xml->GetRoot();

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

void Actor::FixedUpdate(float timeStep)
{
    Vector3 moveTo;
    if (creatureState_ == AB::GameProtocol::CreatureStateMoving)
    {
        float p[3];
        FwClient* c = context_->GetSubsystem<FwClient>();
        double diff = (double)c->GetLastPing();
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
    if (moveToPos_ != Vector3::ZERO && moveToPos_ != cp)
    {
        // Try to make moves smoother...
        if ((cp - moveToPos_).LengthSquared() > 0.01f)
        {
            // Seems to be the best result
            Vector3 pos = cp.Lerp(moveToPos_, 0.4f);
            node_->SetPosition(pos);
        }
        else
            node_->SetPosition(moveToPos_);
    }

    const Quaternion& rot = node_->GetRotation();
    if (rotateTo_ != rot)
    {
        if (fabs(rotateTo_.YawAngle() - rot.YawAngle()) > 0.1f)
        {
            Quaternion r = rot.Slerp(rotateTo_, 0.3f);
            node_->SetRotation(r);
        }
        else
            node_->SetRotation(rotateTo_);
    }
}

void Actor::Update(float timeStep)
{
    Shortcuts* sc = GetSubsystem<Shortcuts>();

    bool highlight = sc->Test(AbEvents::E_SC_HIGHLIGHTOBJECTS);
    if (hovered_ || playerSelected_ || highlight)
    {
        const Vector3& pos = node_->GetPosition();
        IntVector2 screenPos = WorldToScreenPoint(pos);
        float sizeFac = 1.0f;
        if (screenPos != IntVector2::ZERO)
        {
            Node* camNode = GetScene()->GetChild("CameraNode");
            if (camNode)
            {
                const Vector3& dist = pos - camNode->GetPosition();
                sizeFac = 10.0f / dist.Length();
            }

            const BoundingBox& bb = model_->GetBoundingBox();
            Vector3 hpPos = pos + bb.Size();
            IntVector2 hpTop = WorldToScreenPoint(hpPos);
            IntVector2 labelPos(screenPos.x_ - nameLabel_->GetWidth() / 2, screenPos.y_);
            nameLabel_->SetPosition(labelPos);

            hpBar_->SetSize((int)(130.f * sizeFac), (int)(18.f * sizeFac));
            IntVector2 ihpPos(screenPos.x_ - hpBar_->GetWidth() / 2, hpTop.y_);
            hpBar_->SetPosition(ihpPos);
        }
    }

    nameLabel_->SetVisible(highlight || hovered_ || playerSelected_);
    hpBar_->SetVisible((hovered_ && objectType_ != ObjectTypeSelf) || playerSelected_);
}

void Actor::MoveTo(int64_t time, const Vector3& newPos)
{
    moveToPos_ = newPos;
    const float p[3] = { moveToPos_.x_, moveToPos_.y_, moveToPos_.z_ };
    posExtrapolator_.AddSample(GetServerTime(time), GetClientTime(), p);
}

void Actor::SetYRotation(float rad, bool updateYaw)
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

    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();
    nameLabel_ = uiRoot->CreateChild<Text>();
    nameLabel_->SetStyle("ActorNameText");
    nameLabel_->SetText(name_);
    nameLabel_->SetVisible(false);
    // TODO: Why not working?
    SubscribeToEvent(nameLabel_, E_CLICK, URHO3D_HANDLER(Actor, HandleNameClicked));

    hpBar_ = uiRoot->CreateChild<ProgressBar>();
    hpBar_->SetShowPercentText(false);
    hpBar_->SetRange(100.0f);
    hpBar_->SetStyle("HealthBar");
    hpBar_->SetSize(100, 20);
    hpBar_->SetValue(50.0f);
    hpBar_->SetVisible(false);
}

void Actor::RemoveActorUI()
{
    UIElement* uiRoot = GetSubsystem<UI>()->GetRoot();
    if (nameLabel_)
    {
        UnsubscribeFromEvent(nameLabel_, E_CLICK);
        uiRoot->RemoveChild(nameLabel_);
        nameLabel_ = SharedPtr<Text>();
    }
    if (hpBar_)
    {
        uiRoot->RemoveChild(hpBar_);
        hpBar_ = SharedPtr<ProgressBar>();
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
    else if (hash == ANIM_SIT)
        result += "Sitting.ani";
    else
        return "";
    return result;
}

void Actor::HandleNameClicked(StringHash eventType, VariantMap& eventData)
{
    if (nameLabel_->IsVisible())
    {
        VariantMap& eData = GetEventDataMap();
        using namespace AbEvents::ActorNameClicked;
        eData[P_SOURCEID] = id_;
        SendEvent(AbEvents::E_ACTORNAMECLICKED, eData);
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

void Actor::PlayAnimation(StringHash animation, bool looped /* = true */, float fadeTime /* = 0.2f */)
{
    if (!animController_)
        return;

    const String& ani = animations_[animation];
    if (!ani.Empty())
    {
        animController_->PlayExclusive(ani, 0, looped, fadeTime);
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
        PlayAnimation(ANIM_RUN, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateUsingSkill:
        break;
    case AB::GameProtocol::CreatureStateAttacking:
        break;
    case AB::GameProtocol::CreatureStateEmote:
        break;
    case AB::GameProtocol::CreatureStateEmoteSit:
        PlayAnimation(ANIM_SIT, true, fadeTime);
        break;
    case AB::GameProtocol::CreatureStateEmoteCry:
        PlayAnimation(ANIM_CRY, false, fadeTime);
        break;
    default:
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
        if (prevState != AB::GameProtocol::CreatureStateEmoteSit)
            fadeTime = 0.0f;
        else
            fadeTime = 0.5f;
        break;
    case AB::GameProtocol::CreatureStateMoving:
    {
        const float p[3] = { moveToPos_.x_, moveToPos_.y_, moveToPos_.z_ };
        posExtrapolator_.Reset(GetServerTime(time), GetClientTime(), p);
        break;
    }
    case AB::GameProtocol::CreatureStateUsingSkill:
        break;
    case AB::GameProtocol::CreatureStateAttacking:
        break;
    case AB::GameProtocol::CreatureStateEmote:
        break;
    case AB::GameProtocol::CreatureStateEmoteSit:
        fadeTime = 0.5f;
        break;
    case AB::GameProtocol::CreatureStateEmoteCry:
        fadeTime = 0.5f;
        break;
    default:
        break;
    }
    PlayStateAnimation(fadeTime);
}

void Actor::Unserialize(PropReadStream& data)
{
    GameObject::Unserialize(data);
    std::string str;
    if (data.ReadString(str))
        name_ = String(str.data(), static_cast<unsigned>(str.length()));
    uint8_t s;
    FwClient* client = GetSubsystem<FwClient>();
    data.Read(level_);
    if (data.Read(s))
        sex_ = static_cast<AB::Entities::CharacterSex>(s);
    {
        uint32_t p;
        data.Read(p);
        profession_ = client->GetProfessionByIndex(p);
    }
    {
        uint32_t p;
        data.Read(p);
        profession2_ = client->GetProfessionByIndex(p);
    }
    data.Read(modelIndex_);
    AddActorUI();
}

void Actor::PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop /* = false */)
{
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