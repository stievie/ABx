#pragma once

#include "GameObject.h"
#include "Extrapolator.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Item.h>
#include <AB/TemplEncoder.h>
#include "HealthBarPlain.h"

using namespace Urho3D;

const float MOVE_FORCE = 0.7f;
const float INAIR_MOVE_FORCE = 0.02f;
const float BRAKE_FORCE = 0.2f;
const float JUMP_FORCE = 6.0f;
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
static const StringHash ANIM_CASTING("Casting");
static const StringHash ANIM_TAUNTING("Taunting");
static const StringHash ANIM_PONDER("Ponder");
static const StringHash ANIM_WAVE("Wave");
static const StringHash ANIM_LAUGH("Laugh");
static const StringHash ANIM_ATTACK("Attack");
static const StringHash ANIM_CHEST_OPENING("ChestOpening");
static const StringHash ANIM_CHEST_CLOSING("ChestClosing");

/// Stop playing current sound
static const StringHash SOUND_NONE("None");
static const StringHash SOUND_FOOTSTEPS("Footsteps");
static const StringHash SOUND_JUMP("Jump");
static const StringHash SOUND_SKILLFAILURE("SkillFailure");
static const StringHash SOUND_DIE("Die");

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
    unsigned adrenaline = 0;
    unsigned overcast = 0;
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
    ~Actor() override;

    static void RegisterObject(Context* context);

    static Actor* CreateActor(uint32_t id, Scene* scene,
        const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state,
        PropReadStream& data);
    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;
    void MoveTo(int64_t time, const Vector3& newPos) override;
    void ForcePosition(int64_t time, const Vector3& newPos) override;
    void SetYRotation(int64_t time, float rad, bool updateYaw) override;
    void RemoveFromScene() override;
    void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState) override;
    void SetSpeedFactor(int64_t time, float value) override;

    void Unserialize(PropReadStream& data) override;
    /// Get position of head or to of the model in world coordinates.
    Vector3 GetHeadPos() const;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init(Scene* scene, const Vector3& position, const Quaternion& rotation,
        AB::GameProtocol::CreatureState state) override;
    bool LoadModel(uint32_t index, const Vector3& position, const Quaternion& rotation);
    bool LoadAreaOfEffect(uint32_t index, const Vector3& position, const Quaternion& rotation);
    /// Add a model like hair armor etc.
    void AddModel(uint32_t itemIndex);
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    void PlaySoundEffect(const StringHash& type, bool loop = false);
    void PlaySoundEffect(const String& fileName, const String& name = String::EMPTY);
    bool LoadSkillTemplate(const std::string& templ);
    void OnSkillError(AB::GameProtocol::SkillError error) override;

    String GetClasses() const;
    String GetClassLevel() const;
    String GetClassLevelName() const;

    void HandlePartyAdded(StringHash eventType, VariantMap& eventData);
    void HandlePartyRemoved(StringHash eventType, VariantMap& eventData);

    Vector<String> materials_;
    // Can pickup this thingy
    bool pickable_;
    Vector3 velocity_;
private:
    SharedPtr<Text> nameLabel_;
    SharedPtr<Window> nameWindow_;
    SharedPtr<Window> speechBubbleWindow_;
    SharedPtr<Text> speechBubbleText_;
    SharedPtr<HealthBarPlain> hpBar_;
    SharedPtr<Text> classLevel_;
    float speechBubbleVisible_;
    void UpdateTransformation();
    void RemoveActorUI();
    void HideSpeechBubble();
    String GetAnimation(const StringHash& hash);
    String GetSoundEffect(const StringHash& hash);
    void UpdateMoveSpeed();
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleAnimationFinished(StringHash eventType, VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEndSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEffectAdded(StringHash eventType, VariantMap& eventData);
    void HandleItemDropped(StringHash eventType, VariantMap& eventData);
    static void SetUIElementSizePos(UIElement* elem, const IntVector2& size, const IntVector2& pos);
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
    Vector3 moveToPos_;
    Quaternion rotateTo_;
    String name_;
    AB::Entities::CharacterSex sex_;
    uint32_t level_;
    AB::Entities::Profession* profession_;
    AB::Entities::Profession* profession2_;
    AB::SkillIndices skills_;
    AB::Attributes attributes_;
    /// Model or effect (in case of AOE) index
    uint32_t modelIndex_;
    AB::Entities::ModelClass modelClass_;
    Extrapolator<3, float> posExtrapolator_;
    ActorStats stats_;
    void AddActorUI();
    void SetSelectedObject(SharedPtr<GameObject> object);
    SharedPtr<GameObject> GetSelectedObject() const { return selectedObject_.Lock(); }
    uint32_t GetSelectedObjectId() const
    {
        if (auto sel = selectedObject_.Lock())
            return sel->id_;
        return 0;
    }
    void PlayAnimation(StringHash animation, bool looped = true, float fadeTime = 0.2f, float speed = 1.0f);
    void PlayStateAnimation(float fadeTime = 0.2f);
    void ShowSpeechBubble(const String& text);

    void ChangeResource(AB::GameProtocol::ResourceType resType, int32_t value);

    bool IsEnemy(Actor* other) const
    {
        if (!other || other->undestroyable_)
            return false;
        return (other->groupId_ != groupId_);
    }
};
