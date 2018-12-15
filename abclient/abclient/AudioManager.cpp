#include "stdafx.h"
#include "AudioManager.h"
#include "AbEvents.h"

AudioManager::AudioManager(Context* context) :
    Object(context),
    playlistDirty_(false),
    multipleMusicTracks_(false),
    multipleAmbientTracks_(true),
    currentIndex_(-1)
{
    SubscribeToEvents();
}

AudioManager::~AudioManager()
{
    UnsubscribeFromAllEvents();
}

void AudioManager::StartMusic()
{
    if (!playlistDirty_ && musicNodes_.Size() > 0)
        // If we are playing a play list and it didn't change, continue with it
        return;

    if (!multipleMusicTracks_)
        StopMusic();
    playlistDirty_ = false;
    String nextTrack = GetNextMusic();
    URHO3D_LOGINFOF("Playing now %s", nextTrack.CString());
    if (!nextTrack.Empty())
        PlaySound(nextTrack, SOUND_MUSIC);
}

void AudioManager::ContinuePlaylist()
{
    if (!multipleMusicTracks_)
        StopMusic();
    String nextTrack = GetNextMusic();
    URHO3D_LOGINFOF("Playing now %s", nextTrack.CString());
    if (!nextTrack.Empty())
        PlaySound(nextTrack, SOUND_MUSIC);
}

void AudioManager::StopMusic()
{
    musicNodes_.Clear();
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
            SubscribeToEvent(node, E_SOUNDFINISHED, URHO3D_HANDLER(AudioManager, HandleSoundFinished));
            if (type == SOUND_MUSIC)
            {
                if (!multipleMusicTracks_)
                    musicNodes_.Clear();
                musicNodes_[filename] = node;
            }
            else if (type == SOUND_AMBIENT)
            {
                sound->SetLooped(true);
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

void AudioManager::HandleSoundFinished(StringHash, VariantMap& eventData)
{
    using namespace SoundFinished;
    Node* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    SoundSource* sound = static_cast<SoundSource*>(eventData[P_SOUNDSOURCE].GetPtr());
    if (sound->GetSoundType() == SOUND_MUSIC)
    {
        for (const auto& nd : musicNodes_)
        {
            if (nd.second_ == node)
            {
                musicNodes_.Erase(nd.first_);
                break;
            }
        }
        String nextTrack = GetNextMusic();
        URHO3D_LOGINFOF("Playing now %s", nextTrack.CString());
        if (!nextTrack.Empty())
            PlaySound(nextTrack, SOUND_MUSIC);
    }
}

String AudioManager::GetNextMusic()
{
    if (playList_.Size() == 0)
        return String::EMPTY;
    ++currentIndex_;
    if (currentIndex_ >= (int)playList_.Size())
        currentIndex_ = 0;
    return playList_[currentIndex_];
}
