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
    int currentIndex_;
    Vector<String> playList_;
    void SubscribeToEvents();
    void HandleAudioPlay(StringHash eventType, VariantMap& eventData);
    void HandleAudioStop(StringHash eventType, VariantMap& eventData);
    void HandleAudioStopAll(StringHash eventType, VariantMap& eventData);
    void HandleSoundFinished(StringHash eventType, VariantMap& eventData);
    String GetNextMusic();
public:
    AudioManager(Context* context);
    ~AudioManager();

    void StartMusic();
    void StopMusic();
    void PlaySound(const String& filename, const String& type = SOUND_EFFECT);
    void AllowMultipleMusicTracks(bool enabled)
    {
        multipleMusicTracks_ = enabled;
    }
    void AllowMultipleAmbientTracks(bool enabled)
    {
        multipleAmbientTracks_ = enabled;
    }
    void SetPlayList(const Vector<String>& playList)
    {
        currentIndex_ = -1;
        playList_ = playList;
    }
};

