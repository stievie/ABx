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

#pragma once

#include "GameObject.h"
#include <AB/Entities/Character.h>
#include <AB/Entities/Profession.h>
#include <AB/Entities/Item.h>
#include <abshared/TemplEncoder.h>
#include "ValueBar.h"
#include <Urho3DAll.h>

using namespace Urho3D;

static const StringHash ANIM_IDLE("Idle");
static const StringHash ANIM_WALK("Walk");
static const StringHash ANIM_RUN("Run");
static const StringHash ANIM_JUMP("Jump");
static const StringHash ANIM_SIT("Sit");
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

struct ActorStats
{
    unsigned health{ 0 };
    unsigned maxHealth{ 0 };
    int healthRegen{ 0 };
    unsigned energy{ 0 };
    unsigned maxEnergy{ 0 };
    int energyRegen{ 0 };
    unsigned adrenaline{ 0 };
    unsigned overcast{ 0 };
    int morale{ 0 };
};

/// Character component, responsible for physical movement according to controls, as well as animation.
class Actor : public GameObject
{
    URHO3D_OBJECT(Actor, GameObject)
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
        const Vector3& position, const Quaternion& rotation, const Vector3& scale,
        AB::GameProtocol::CreatureState state,
        sa::PropReadStream& data);
    /// Handle physics world update. Called by LogicComponent base class.
    void Update(float timeStep) override;
    void MoveTo(int64_t time, const Vector3& newPos) override;
    void ForcePosition(int64_t time, const Vector3& newPos) override;
    void SetYRotation(int64_t time, float rad, bool updateYaw) override;
    void RemoveFromScene() override;
    void SetCreatureState(int64_t time, AB::GameProtocol::CreatureState newState) override;
    void SetSpeedFactor(int64_t time, float value) override;

    void Unserialize(sa::PropReadStream& data) override;
    /// Get position of head or to of the model in world coordinates.
    Vector3 GetHeadPos() const;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init(Scene* scene, const Vector3& position, const Quaternion& rotation, const Vector3& scale,
        AB::GameProtocol::CreatureState state) override;
    bool LoadObject(uint32_t itemIndex, const Vector3& position, const Quaternion& rotation, const Vector3 scale);
    /// Add a model like hair armor etc.
    void AddModel(uint32_t itemIndex);
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    void PlaySoundEffect(const StringHash& type, bool loop = false);
    void PlaySoundEffect(const String& fileName, const String& name = String::EMPTY);
    bool LoadSkillTemplate(const std::string& templ);
    std::string SaveSkillTemplate();
    void OnSkillError(AB::GameProtocol::SkillError error) override;

    String GetClasses() const;
    String GetClassLevel() const;
    String GetClassLevelName() const;

    void HandlePartyAdded(StringHash eventType, VariantMap& eventData);
    void HandlePartyRemoved(StringHash eventType, VariantMap& eventData);

    uint32_t GetAttributeRank(Game::Attribute index) const;
    void SetAttributeRank(Game::Attribute index, uint32_t value);

    Vector<String> materials_;
    Vector3 velocity_;
    bool autoRun_{ false };
    bool pvpCharacter_{ false };
private:
    bool hovered_{ false };
    SharedPtr<Text> nameLabel_;
    SharedPtr<Window> nameWindow_;
    SharedPtr<Window> speechBubbleWindow_;
    SharedPtr<Text> speechBubbleText_;
    SharedPtr<ValueBar> hpBar_;
    SharedPtr<Text> classLevel_;
    float speechBubbleVisible_{ false };
    void RemoveActorUI();
    void HideSpeechBubble();
    String GetAnimation(const StringHash& hash);
    String GetSoundEffect(const StringHash& hash);
    void UpdateMoveSpeed();
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleNameDoubleClicked(StringHash eventType, VariantMap& eventData);
    void HandleAnimationFinished(StringHash eventType, VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEndSkillUse(StringHash eventType, VariantMap& eventData);
    void HandleEffectAdded(StringHash eventType, VariantMap& eventData);
    void HandleItemDropped(StringHash eventType, VariantMap& eventData);
    void HandleObjectSecProfessionChange(StringHash eventType, VariantMap& eventData);
    void HandleLoadSkillTemplate(StringHash eventType, VariantMap& eventData);
    void HandleSetAttribValue(StringHash eventType, VariantMap& eventData);
    void HandleSetSkill(StringHash eventType, VariantMap& eventData);
    void HandleGroupMaskChanged(StringHash eventType, VariantMap& eventData);
    void HandleSetAttackSpeed(StringHash eventType, VariantMap& eventData);
    void HandleDropTargetChanged(StringHash eventType, VariantMap& eventData);
    static void SetUIElementSizePos(UIElement* elem, const IntVector2& size, const IntVector2& pos);
    bool IsSpeechBubbleVisible() const;
protected:
    AnimatedModel* animatedModel_{ nullptr };
    Actor::ModelType type_{ ModelType::Static };
    float attackSpeed_{ 1.0f };
    SharedPtr<AnimationController> animController_;
    SharedPtr<StaticModel> model_;
    HashMap<StringHash, String> animations_;
    /// Footsteps etc.
    HashMap<StringHash, String> sounds_;
    WeakPtr<GameObject> selectedObject_;
    StringHash currentAnimation_;
    static String GetClassSubdir(AB::Entities::ModelClass cls);
public:
    static String GetAnimation(AB::Entities::ModelClass cls, const StringHash& hash);
    String name_;
    AB::Entities::CharacterSex sex_{ AB::Entities::CharacterSex::Unknown };
    uint32_t level_{ 0 };
    AB::Entities::Profession* profession_{ nullptr };
    AB::Entities::Profession* profession2_{ nullptr };
    Game::SkillIndices skills_;
    Game::Attributes attributes_;
    /// Model or effect (in case of AOE) index
    uint32_t itemIndex_;
    // If this is an item drop, the ID of the player for who it dropped
    uint32_t dropTarget_{ 0 };
    AB::Entities::ModelClass modelClass_;
    ActorStats stats_;
    void SetMoveToPos(const Vector3& pos);
    const Vector3& GetMoveToPos() const;
    void SetRotateTo(const Quaternion& rot);
    const Quaternion& GetRotateTo() const;
    void ResetSecondProfAttributes();
    bool IsDead() const { return stats_.health == 0; }
    void AddActorUI();
    void SetSelectedObject(SharedPtr<GameObject> object);
    AnimatedModel* GetModel() const { return animatedModel_; }
    GameObject* GetSelectedObject() const
    {
        if (auto sel = selectedObject_.Lock())
            return sel.Get();
        return nullptr;
    }
    uint32_t GetSelectedObjectId() const
    {
        if (auto sel = selectedObject_.Lock())
            return sel->gameId_;
        return 0;
    }
    void PlayAnimation(StringHash animation, bool looped = true, float fadeTime = 0.2f, float speed = 1.0f);
    void PlayObjectAnimation(bool looped = false, float fadeTime = 0.2f, float speed = 1.0f);
    void PlayIdleAnimation(float fadeTime);
    void PlayStateAnimation(float fadeTime = 0.2f);
    void ShowSpeechBubble(const String& text);
    int GetAttributePoints() const;
    int GetUsedAttributePoints() const;
    int GetAvailableAttributePoints() const;
    bool CanIncreaseAttributeRank(Game::Attribute index) const;

    void ChangeResource(AB::GameProtocol::ResourceType resType, int32_t value);

    /// Get lower 16 bits of the group mask
    uint32_t GetFriendMask() const { return groupMask_ & 0xffff; }
    /// Get upper 16 bits of the group mask
    uint32_t GetFoeMask() const { return groupMask_ >> 16; }

    bool IsEnemy(const Actor* other) const
    {

        if (!other || other->undestroyable_)
            return false;

        if (groupId_ != 0 && groupId_ == other->groupId_)
            // Return true if we have a matching bit of our foe mask in their friend mask
            return false;
        // Return true if we have a matching bit of our foe mask in their friend mask
        return ((GetFoeMask() & other->GetFriendMask()) != 0);
    }
    bool IsAlly(const Actor* other) const
    {
        if (!other || other->undestroyable_)
            return false;
        if (groupId_ != 0 && groupId_ == other->groupId_)
            // Same group members are always friends
            return true;
        // Return true if we have a matching bit of our foe mask in their friend mask
        return ((GetFriendMask() & other->GetFriendMask()) != 0);
    }
    void HoverBegin() { hovered_ = true; }
    void HoverEnd() { hovered_ = false; }
};

template <>
inline bool Is<Actor>(const GameObject& obj)
{
    return obj.objectType_ > ObjectType::Static;
}
