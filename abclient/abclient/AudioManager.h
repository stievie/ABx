#pragma once

#include <AB/Entities/Music.h>

/// Only for non-3D sounds, like ambient and music.
/// !!! Don't use MP3 use OGG instead. !!!
class AudioManager : public Object
{
    URHO3D_OBJECT(AudioManager, Object);
private:
    bool playlistDirty_;
    bool multipleMusicTracks_;
    bool multipleAmbientTracks_;
    HashMap<String, SharedPtr<Node>> musicNodes_;
    HashMap<String, SharedPtr<Node>> ambientNodes_;
    int currentIndex_;
    Vector<String>* playList_;
    HashMap<String, Vector<String>> musicList_;
    HashMap<String, AB::Entities::MusicStyle> musicStyles_;
    void SubscribeToEvents();
    void HandleAudioPlay(StringHash eventType, VariantMap& eventData);
    void HandleAudioStop(StringHash eventType, VariantMap& eventData);
    void HandleAudioStopAll(StringHash eventType, VariantMap& eventData);
    void HandlePlayMapMusic(StringHash eventType, VariantMap& eventData);
    void HandlePlayMusicStyle(StringHash eventType, VariantMap& eventData);
    void HandleSoundFinished(StringHash eventType, VariantMap& eventData);
    const String& GetNextMusic();
    Vector<String>* GetMapPlaylist(const String& mapUuid);
    /// Get a music file with a certain file
    const String& GetMusicWidthStyle(AB::Entities::MusicStyle style);
    bool IsPlayingFile(const String& file) const;
    void SetPlayList(Vector<String>* playList)
    {
        if (!playList)
        {
            playList_ = nullptr;
            return;
        }
        if (!playList_)
        {
            playList_ = playList;
            return;
        }

        if (playList_->Size() == playList->Size())
        {
            bool equal = true;
            for (unsigned i = 0; i < playList->Size(); ++i)
            {
                if ((*playList_)[i].Compare((*playList)[i]) != 0)
                {
                    equal = false;
                    break;
                }
            }
            if (equal)
                return;
        }
        currentIndex_ = -1;
        playList_ = playList;
        playlistDirty_ = true;
    }
public:
    AudioManager(Context* context);
    ~AudioManager();

    void LoadMusic(XMLFile* file);
    void StartMusic();
    void ContinuePlaylist();
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
    void SetMapPlayList(const String& mapUuid)
    {
        SetPlayList(GetMapPlaylist(mapUuid));
    }
};

