#pragma once

#include "GameObject.h"

using namespace Urho3D;

const float MOVE_FORCE = 0.7f;
const float INAIR_MOVE_FORCE = 0.02f;
const float BRAKE_FORCE = 0.2f;
const float JUMP_FORCE = 6.0f;
const float YAW_SENSITIVITY = 0.1f;
const float INAIR_THRESHOLD_TIME = 0.1f;

static const StringHash ANIM_IDLE("Idle");
static const StringHash ANIM_WALK("Walk");
static const StringHash ANIM_RUN("Run");
static const StringHash ANIM_JUMP("Jump");
static const StringHash ANIM_ATTACK_MELEE("Melee");
static const StringHash ANIM_ATTACK_PISTOL("Shoot Pistol");
static const StringHash ANIM_ATTACK_GUN("Shoot Gun");
static const StringHash ANIM_HURT("Hurt");
static const StringHash ANIM_DYING("Dying");
static const StringHash ANIM_DEAD("Dead");

/// Stop playing current sound
static const StringHash SOUND_NONE("None");
static const StringHash SOUND_FOOTSTEPS("Footsteps");
static const StringHash SOUND_JUMP("Jump");

static const StringHash COLLADJ_ADD("add");
static const StringHash COLLADJ_SUB("sub");
static const StringHash COLLADJ_MUL("mul");
static const StringHash COLLADJ_DIV("div");

/// Character component, responsible for physical movement according to controls, as well as animation.
class Actor : public GameObject
{
    URHO3D_OBJECT(Actor, GameObject);
public:
    enum ModelType {
        Static,
        Animated
    };
public:
    /// Construct.
    Actor(Context* context);
    ~Actor();

    static void RegisterObject(Context* context);

    static Actor* CreateActor(uint32_t id, Context* context, Scene* scene);
    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;

    void Unserialize(PropReadStream& data) override;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init() override;
    void LoadXML(const XMLElement& source);
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    /// Model file name
    String mesh_;
    Vector<String> materials_;
    // Can pickup this thingy
    bool pickable_;
    bool castShadows_;
private:
    void CreateModel();
protected:
    AnimatedModel* animatedModel_;
    Actor::ModelType type_;
    SharedPtr<AnimationController> animController_;
    SharedPtr<StaticModel> model_;
    HashMap<StringHash, String> animations_;
    /// Footsteps etc.
    HashMap<StringHash, String> sounds_;
public:
    StaticModel* GetModel() const { return model_; }
    WeakPtr<GameObject> selectedObject_;
    String name_;
};
