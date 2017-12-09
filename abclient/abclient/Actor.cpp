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

// Most of it taken from the Character Demo

#include "stdafx.h"
#include "Actor.h"

Actor::Actor(Context* context) :
    LogicComponent(context),
    pickable_(false),
    castShadows_(true),
    mesh_(String::EMPTY),
    animController_(nullptr),
    model_(nullptr)
{
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

Actor::~Actor()
{
}

void Actor::Init()
{
    if (!mesh_.Empty())
    {
        CreateModel();
    }
}

void Actor::CreateModel()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // spin node
    Node* adjustNode = node_->CreateChild("AdjNode");
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
}

void Actor::FixedUpdate(float timeStep)
{
    // Update movement & animation
    const Quaternion& rot = node_->GetRotation();
    Vector3 moveDir = Vector3::ZERO;

    // Normalize move vector so that diagonal strafing is not faster
    if (moveDir.LengthSquared() > 0.0f)
        moveDir.Normalize();
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