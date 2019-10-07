#include "stdafx.h"
#include "AudioManager.h"
#include <Urho3D/ThirdParty/PugiXml/pugixml.hpp>
#include <AB/Entities/Music.h>

AudioManager::AudioManager(Context* context) :
    Object(context),
    playlistDirty_(false),
    multipleAmbientTracks_(true),
    currentIndex_(-1),
    playList_(nullptr),
    musicStream_(nullptr)
{
    SubscribeToEvents();
}

AudioManager::~AudioManager()
{
    UnsubscribeFromAllEvents();
}

void AudioManager::LoadMusic(XMLFile* file)
{
    if (!file)
        return;

    const pugi::xml_document* const doc = file->GetDocument();
    const pugi::xml_node& node = doc->child("music_list");
    if (!node)
        return;

    for (const auto& pro : node.children("music"))
    {
        // <uuid1>;<uuid2>...
        String mapStr(pro.attribute("map_uuid").as_string());
        StringVector maps = mapStr.Split(';');
        String localFile(pro.attribute("local_file").as_string());
        for (const auto& map : maps)
            musicList_[map].Push(localFile);
        musicStyles_[localFile] = static_cast<AB::Entities::MusicStyle>(pro.attribute("style").as_uint());
    }
}

Vector<String>* AudioManager::GetMapPlaylist(const String& mapUuid)
{
    if (musicList_.Contains(mapUuid))
        return &musicList_[mapUuid];
    if (musicList_.Contains("00000000-0000-0000-0000-000000000000"))
        return &musicList_["00000000-0000-0000-0000-000000000000"];
    return nullptr;
}

const String& AudioManager::GetMusicWidthStyle(AB::Entities::MusicStyle style)
{
    if (!playList_)
        return String::EMPTY;

    for (const auto& file : *playList_)
    {
        if ((musicStyles_[file] & style) == style)
            return file;
    }
    return String::EMPTY;
}

bool AudioManager::IsPlayingFile(const String& file) const
{
    for (const auto& nd : musicNodes_)
    {
        if (nd.first_.Compare(file) == 0)
            return true;
    }
    for (const auto& nd : ambientNodes_)
    {
        if (nd.first_.Compare(file) == 0)
            return true;
    }
    return false;
}

void AudioManager::StartMusic()
{
    if (!playlistDirty_ && musicNodes_.Size() > 0)
        // If we are playing a play list and it didn't change, continue with it
        return;

    StopMusic();
    playlistDirty_ = false;
    const String& nextTrack = GetNextMusic();
    URHO3D_LOGINFOF("Playing now %s", nextTrack.CString());
    if (!nextTrack.Empty())
        PlaySound(nextTrack, SOUND_MUSIC);
}

void AudioManager::ContinuePlaylist()
{
    StopMusic();
    const String& nextTrack = GetNextMusic();
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
    Sound* sound = cache->GetResource<Sound>(filename);
    if (sound)
    {
        Node* node = new Node(context_);
        // Create a SoundSource component for playing the sound. The SoundSource component plays
        // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
        // SoundSource3D component would be used instead
        auto* soundSource = node->CreateComponent<SoundSource>();
        soundSource->SetSoundType(type);
        if (type == SOUND_MUSIC)
            musicStream_.Reset();
        if (type == SOUND_EFFECT || type == SOUND_VOICE)
            // Component will automatically remove itself when the sound finished playing
            soundSource->SetAutoRemoveMode(REMOVE_NODE);
        else
        {
            SubscribeToEvent(node, E_SOUNDFINISHED, URHO3D_HANDLER(AudioManager, HandleSoundFinished));
            if (type == SOUND_MUSIC)
            {
                musicNodes_.Clear();
                musicNodes_[filename] = node;
                if (filename.EndsWith(".ogg", false))
                {
                    musicStream_ = SharedPtr<OggVorbisSoundStream>(new OggVorbisSoundStream(sound));
                }
            }
            else if (type == SOUND_AMBIENT)
            {
                sound->SetLooped(true);
                if (!multipleAmbientTracks_)
                    ambientNodes_.Clear();
                ambientNodes_[filename] = node;
            }
        }
        if (musicStream_ && type == SOUND_MUSIC)
            soundSource->Play(musicStream_);
        else
            soundSource->Play(sound);
    }
}

void AudioManager::SubscribeToEvents()
{
    SubscribeToEvent(Events::E_AUDIOPLAY, URHO3D_HANDLER(AudioManager, HandleAudioPlay));
    SubscribeToEvent(Events::E_AUDIOSTOP, URHO3D_HANDLER(AudioManager, HandleAudioStop));
    SubscribeToEvent(Events::E_AUDIOSTOPALL, URHO3D_HANDLER(AudioManager, HandleAudioStopAll));
    SubscribeToEvent(Events::E_AUDIOPLAYMAPMUSIC, URHO3D_HANDLER(AudioManager, HandlePlayMapMusic));
    SubscribeToEvent(Events::E_AUDIOPLAYMUSICSTYLE, URHO3D_HANDLER(AudioManager, HandlePlayMusicStyle));
}

void AudioManager::HandleAudioPlay(StringHash, VariantMap& eventData)
{
    using namespace Events::AudioPlay;
    const String& name = eventData[P_NAME].GetString();
    const String& type = eventData[P_TYPE].GetString();
    PlaySound(name, type);
}

void AudioManager::HandleAudioStop(StringHash, VariantMap& eventData)
{
    using namespace Events::AudioStop;
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
        musicStream_.Reset();
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
    musicStream_.Reset();
    ambientNodes_.Clear();
}

void AudioManager::HandlePlayMapMusic(StringHash, VariantMap& eventData)
{
    using namespace Events::AudioPlayMapMusic;
    const String& uuid = eventData[P_MAPUUID].GetString();
    SetMapPlayList(uuid);
    StartMusic();
}

void AudioManager::HandlePlayMusicStyle(StringHash, VariantMap& eventData)
{
    using namespace Events::AudioPlayMusicStyle;
    AB::Entities::MusicStyle style = static_cast<AB::Entities::MusicStyle>(eventData[P_STYLE].GetUInt());
    if (style == AB::Entities::MusicStyleUnknown)
        return;
    const String& file = GetMusicWidthStyle(style);
    if (!file.Empty() && !IsPlayingFile(file))
        PlaySound(file, SOUND_MUSIC);
}

void AudioManager::HandleSoundFinished(StringHash, VariantMap& eventData)
{
    using namespace SoundFinished;
    Node* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
    SoundSource* sound = static_cast<SoundSource*>(eventData[P_SOUNDSOURCE].GetPtr());
    if (sound->GetSoundType() == SOUND_MUSIC)
    {
        musicStream_.Reset();
        for (const auto& nd : musicNodes_)
        {
            if (nd.second_ == node)
            {
                musicNodes_.Erase(nd.first_);
                break;
            }
        }
        const String& nextTrack = GetNextMusic();
        URHO3D_LOGINFOF("Playing now %s", nextTrack.CString());
        if (!nextTrack.Empty())
            PlaySound(nextTrack, SOUND_MUSIC);
    }
}

const String& AudioManager::GetNextMusic()
{
    if (!playList_)
        return String::EMPTY;

    if (playList_->Size() == 0)
        return String::EMPTY;
    ++currentIndex_;
    if (currentIndex_ >= (int)playList_->Size())
        currentIndex_ = 0;
    return (*playList_)[currentIndex_];
}
