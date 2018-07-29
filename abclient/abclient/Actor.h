#pragma once

#include "GameObject.h"
#include "Extrapolator.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>

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
static const StringHash ANIM_SIT("Sit");
static const StringHash ANIM_ATTACK_MELEE("Melee");
static const StringHash ANIM_ATTACK_PISTOL("Shoot Pistol");
static const StringHash ANIM_ATTACK_GUN("Shoot Gun");
static const StringHash ANIM_HURT("Hurt");
static const StringHash ANIM_DYING("Dying");
static const StringHash ANIM_DEAD("Dead");
static const StringHash ANIM_CRY("Cry");

/// Stop playing current sound
static const StringHash SOUND_NONE("None");
static const StringHash SOUND_FOOTSTEPS("Footsteps");
static const StringHash SOUND_JUMP("Jump");

static const StringHash COLLADJ_ADD("add");
static const StringHash COLLADJ_SUB("sub");
static const StringHash COLLADJ_MUL("mul");
static const StringHash COLLADJ_DIV("div");

struct ActorStats
{
    unsigned health = 0;
    unsigned maxHealth = 0;
    int healthRegen = 0;
    unsigned energy = 0;
    unsigned maxEnergy = 0;
    int energyRegen = 0;
};

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

    static Actor* CreateActor(uint32_t id, Context* context, Scene* scene,
        const Vector3& position, const Quaternion& rotation,
        PropReadStream& data);
    /// Handle physics world update. Called by LogicComponent base class.
    virtual void FixedUpdate(float timeStep) override;
    virtual void Update(float timeStep) override;
    void MoveTo(int64_t time, const Vector3& newPos) override;
    void SetYRotation(float rad, bool updateYaw) override;
    void RemoveFromScene() override;
    void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState) override;

    void Unserialize(PropReadStream& data) override;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init(Scene* scene, const Vector3& position, const Quaternion& rotation) override;
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    Vector<String> materials_;
    // Can pickup this thingy
    bool pickable_;
    bool castShadows_;
private:
    SharedPtr<Text> nameLabel_;
    SharedPtr<ProgressBar> hpBar_;
    void AddActorUI();
    void RemoveActorUI();
    String GetAnimation(const StringHash& hash);
protected:
    AnimatedModel* animatedModel_;
    Actor::ModelType type_;
    SharedPtr<AnimationController> animController_;
    SharedPtr<StaticModel> model_;
    HashMap<StringHash, String> animations_;
    /// Footsteps etc.
    HashMap<StringHash, String> sounds_;
    WeakPtr<GameObject> selectedObject_;
public:
    StaticModel* GetModel() const { return model_; }
    Vector3 moveToPos_;
    Quaternion rotateTo_;
    String name_;
    AB::Entities::CharacterSex sex_;
    uint32_t level_;
    AB::Entities::Profession* profession_;
    AB::Entities::Profession* profession2_;
    uint32_t modelIndex_;
    Extrapolator<3, float> posExtrapolator_;
    ActorStats stats_;
    void SetSelectedObject(SharedPtr<GameObject> object);
    SharedPtr<GameObject> GetSelectedObject() const { return selectedObject_.Lock(); }
    void PlayAnimation(StringHash animation, bool looped = true, float fadeTime = 0.2f);
};
