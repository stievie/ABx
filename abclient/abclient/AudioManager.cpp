#include "stdafx.h"
#include "AudioManager.h"
#include "AbEvents.h"

AudioManager::AudioManager(Context* context) :
    Object(context),
    multipleMusicTracks_(true),
    multipleAmbientTracks_(true)
{
    SubscribeToEvents();
}

AudioManager::~AudioManager()
{
    UnsubscribeFromAllEvents();
}

void AudioManager::PlaySound(const String& filename, const String& type)
{
    if (filename.Empty())
        return;

    // Get the sound resource
    auto* cache = GetSubsystem<ResourceCache>();
    auto* sound = cache->GetResource<Sound>(filename);
    if (sound)
    {
        Node* node = new Node(context_);
        // Create a SoundSource component for playing the sound. The SoundSource component plays
        // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
        // SoundSource3D component would be used instead
        auto* soundSource = node->CreateComponent<SoundSource>();
        if (type == SOUND_EFFECT || type == SOUND_VOICE)
            // Component will automatically remove itself when the sound finished playing
            soundSource->SetAutoRemoveMode(REMOVE_NODE);
        else
        {
            sound->SetLooped(true);
            if (type == SOUND_MUSIC)
            {
                if (!multipleMusicTracks_)
                    musicNodes_.Clear();
                musicNodes_[filename] = node;
            }
            else if (type == SOUND_AMBIENT)
            {
                if (!multipleMusicTracks_)
                    ambientNodes_.Clear();
                ambientNodes_[filename] = node;
            }
        }

        soundSource->SetSoundType(type);
        soundSource->Play(sound);
    }
}

void AudioManager::SubscribeToEvents()
{
    SubscribeToEvent(AbEvents::E_AUDIOPLAY, URHO3D_HANDLER(AudioManager, HandleAudioPlay));
    SubscribeToEvent(AbEvents::E_AUDIOSTOP, URHO3D_HANDLER(AudioManager, HandleAudioStop));
    SubscribeToEvent(AbEvents::E_AUDIOSTOPALL, URHO3D_HANDLER(AudioManager, HandleAudioStopAll));
}

void AudioManager::HandleAudioPlay(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::AudioPlay;
    const String& name = eventData[P_NAME].GetString();
    const String& type = eventData[P_TYPE].GetString();
    PlaySound(name, type);
}

void AudioManager::HandleAudioStop(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::AudioStop;
    const String& name = eventData[P_NAME].GetString();
    const String& type = eventData[P_TYPE].GetString();

    if (type == SOUND_EFFECT)
    {
    }
    else if (type == SOUND_MASTER)
    {
    }
    else if (type == SOUND_AMBIENT)
    {
        // Disable only specific music
        if (ambientNodes_[name])
            ambientNodes_.Erase(name);
        else if (name.Empty())
        // Disable all music
            ambientNodes_.Clear();
    }
    if (type == SOUND_VOICE)
    {
    }
    else if (type == SOUND_MUSIC)
    {
        // Disable only specific music
        if (musicNodes_[name])
            musicNodes_.Erase(name);
        else if (name.Empty())
            // Disable all music
            musicNodes_.Clear();
    }
}

void AudioManager::HandleAudioStopAll(StringHash, VariantMap&)
{
    musicNodes_.Clear();
    ambientNodes_.Clear();
}
