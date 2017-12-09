//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

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
class Actor : public LogicComponent
{
    URHO3D_OBJECT(Actor, LogicComponent);
public:
    enum ModelType {
        Static,
        Animated
    };
public:
    /// Construct.
    Actor(Context* context);
    ~Actor();

    /// Handle startup. Called by LogicComponent base class.
    virtual void Start() {}
    /// Handle physics world update. Called by LogicComponent base class.
    virtual void FixedUpdate(float timeStep);

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    virtual void Init();
    void LoadXML(const XMLElement& source);
    void PlaySoundEffect(SoundSource3D* soundSource, const StringHash& type, bool loop = false);
    /// Movement controls. Assigned by the main program each frame.
    Controls controls_;
    /// Model file name
    String mesh_;
    Vector<String> materials_;
    // Can pickup this thingy
    bool pickable_;
    bool castShadows_;
private:
    void CreateModel();
protected:
    Actor::ModelType type_;
    SharedPtr<AnimationController> animController_;
    SharedPtr<StaticModel> model_;
    HashMap<StringHash, String> animations_;
    /// Footsteps etc.
    HashMap<StringHash, String> sounds_;
public:
    StaticModel* GetModel() const { return model_; }
};
