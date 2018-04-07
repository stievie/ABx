// Most of it taken from the Character Demo

#include "stdafx.h"
#include "Actor.h"
#include "Definitions.h"
#include <SDL/SDL_timer.h>
#include "LevelManager.h"
#include "BaseLevel.h"
#include "MathUtils.h"
#include "TimeUtils.h"
#include <AB/ProtocolCodes.h>
#include "FwClient.h"

#include <Urho3D/DebugNew.h>

Actor::Actor(Context* context) :
    GameObject(context),
    pickable_(false),
    castShadows_(true),
    mesh_(String::EMPTY),
    animController_(nullptr),
    model_(nullptr),
    selectedObject_(nullptr),
    nameLabel_(nullptr)
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

Actor* Actor::CreateActor(uint32_t id, Context* context, Scene* scene)
{
    Node* objectNode = scene->CreateChild();
    Actor* result = objectNode->CreateComponent<Actor>();
    result->id_ = id;

    Node* adjustNode = result->GetNode()->CreateChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));

    result->Init();
    adjustNode->CreateComponent<AnimationController>();
    result->animatedModel_ = adjustNode->CreateComponent<AnimatedModel>();
    result->animatedModel_->SetCastShadows(true);
    adjustNode->CreateComponent<AnimationController>();

    return result;

}

void Actor::Init()
{
    mesh_ = "Models/Sphere.mdl";
    materials_.Push("Materials/Stone.xml");
    if (!mesh_.Empty())
    {
        CreateModel();
    }
}

void Actor::CreateModel()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Create rigidbody, and set non-zero mass so that the body becomes dynamic
    RigidBody* body = GetNode()->CreateComponent<RigidBody>();
    body->SetMass(1.0f);
    // Set zero angular factor so that physics doesn't turn the character on its own.
    // Instead we will control the character yaw manually
    body->SetAngularFactor(Vector3::ZERO);
    body->SetLinearFactor(Vector3(1.0f, 0.0f, 1.0f));
    body->SetCollisionMask(0);

    // spin node
    Node* adjustNode = GetNode()->GetChild("AdjNode");
    adjustNode->SetRotation(Quaternion(180, Vector3(0, 1, 0)));

    // Create the rendering component + animation controller
    if (type_ == Actor::Animated)
    {
        AnimatedModel* animModel = adjustNode->CreateComponent<AnimatedModel>();
        animModel->SetModel(cache->GetResource<Model>(mesh_));
        model_ = animModel;
        animController_ = adjustNode->CreateComponent<AnimationController>();
    }
    else
    {
        model_ = adjustNode->CreateComponent<StaticModel>();
        model_->SetModel(cache->GetResource<Model>(mesh_));
    }
    int i = 0;
    for (Vector<String>::ConstIterator it = materials_.Begin(); it != materials_.End(); it++, i++)
    {
        model_->SetMaterial(i, cache->GetResource<Material>((*it)));
    }
    model_->SetCastShadows(castShadows_);

    CollisionShape* shape = node_->CreateComponent<CollisionShape>();
    const BoundingBox& bb = model_->GetBoundingBox();
    shape->SetCylinder(bb.Size().x_, bb.Size().y_);
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

    const Vector3& cp = GetNode()->GetPosition();
    if (moveToPos_ != Vector3::ZERO && moveToPos_ != cp)
    {
        // Try to make moves smoother...
        if ((cp - moveToPos_).LengthSquared() > 0.01f)
        {
            // Seems to be the best result
            Vector3 pos = cp.Lerp(moveToPos_, 0.4f);
            GetNode()->SetPosition(pos);
        }
        else
            GetNode()->SetPosition(moveToPos_);
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
    Input* input = GetSubsystem<Input>();
    bool isLCtrlDown = input->GetScancodeDown(SDL_SCANCODE_LCTRL);
    if (hovered_ || playerSelected_ || isLCtrlDown)
    {
        Vector3 pos = node_->GetPosition();
        IntVector2 screenPos = WorldToScreenPoint(pos);
        float sizeFac = 1.0f;
        if (screenPos != IntVector2::ZERO)
        {
            Node* camNode = GetScene()->GetChild("CameraNode");
            if (camNode)
            {
                Vector3 dist = pos - camNode->GetPosition();
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

    nameLabel_->SetVisible(isLCtrlDown || hovered_ || playerSelected_);
    hpBar_->SetVisible(hovered_ || playerSelected_);
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
        uiRoot->RemoveChild(nameLabel_);
    if (hpBar_)
        uiRoot->RemoveChild(hpBar_);
}

void Actor::SelectObject(SharedPtr<GameObject> object)
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

void Actor::SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState)
{
    if (creatureState_ == AB::GameProtocol::CreatureStateMoving)
    {
        const float p[3] = { moveToPos_.x_, moveToPos_.y_, moveToPos_.z_ };
        posExtrapolator_.Reset(GetServerTime(time), GetClientTime(), p);
    }
    GameObject::SetCreatureState(time, newState);
}

void Actor::Unserialize(PropReadStream& data)
{
    GameObject::Unserialize(data);
    std::string str;
    if (data.ReadString(str))
        name_ = String(str.data(), (unsigned)str.length());

    AddActorUI();
}

void Actor::LoadXML(const XMLElement& source)
{
    assert(source.GetName() == "actor");

    String typeName = source.GetAttribute("type");
    if (typeName.Compare("Animated") == 0)
    {
        type_ = Actor::Animated;
    }
    else
    {
        type_ = Actor::Static;
    }
    if (source.HasAttribute("pickable"))
        pickable_ = source.GetBool("pickable");
    if (source.HasAttribute("castshadows"))
        castShadows_ = source.GetBool("castshadows");

    // Read the mesh
    XMLElement meshElem = source.GetChild("mesh");
    // We need a mesh
    assert(meshElem);
    mesh_ = meshElem.GetValue();

    // Get materials
    XMLElement materialsElem = source.GetChild("materials");
    if (materialsElem)
    {
        materials_.Clear();
        XMLElement matElem = materialsElem.GetChild("material");
        while (matElem)
        {
            materials_.Push(matElem.GetValue());
            matElem = matElem.GetNext("material");
        }
    }

    // If animated model, get animations if any
    if (type_ == Actor::Animated)
    {
        XMLElement animsElem = source.GetChild("animations");
        if (animsElem)
        {
            XMLElement aniElem = animsElem.GetChild("animation");
            while (aniElem)
            {
                String aniType = aniElem.GetAttribute("type");
                animations_[StringHash(aniType)] = aniElem.GetValue();

                aniElem = aniElem.GetNext("animation");
            }
        }
    }

    // Sounds
    XMLElement soundsElem = source.GetChild("sounds");
    if (soundsElem)
    {
        XMLElement soundElem = soundsElem.GetChild("sound");
        while (soundElem)
        {
            String soundType = soundElem.GetAttribute("type");
            sounds_[StringHash(soundType)] = soundElem.GetValue();
            soundElem = soundElem.GetNext("sound");
        }
    }
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