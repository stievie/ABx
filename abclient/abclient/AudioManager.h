#pragma once

/// Only for non-3D sounds, like ambient and music.
/// !!! Don't use MP3 use OGG instead. !!!
class AudioManager : public Object
{
    URHO3D_OBJECT(AudioManager, Object);
private:
    bool multipleMusicTracks_;
    bool multipleAmbientTracks_;
    HashMap<String, SharedPtr<Node>> musicNodes_;
    HashMap<String, SharedPtr<Node>> ambientNodes_;
    void SubscribeToEvents();
    void HandleAudioPlay(StringHash eventType, VariantMap& eventData);
    void HandleAudioStop(StringHash eventType, VariantMap& eventData);
    void HandleAudioStopAll(StringHash eventType, VariantMap& eventData);
public:
    AudioManager(Context* context);
    ~AudioManager();

    void PlaySound(const String& filename, const String& type = SOUND_EFFECT);
    void AllowMultipleMusicTracks(bool enabled)
    {
        multipleMusicTracks_ = enabled;
    }
    void AllowMultipleAmbientTracks(bool enabled)
    {
        multipleAmbientTracks_ = enabled;
    }
};

