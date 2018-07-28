#pragma once

class Options : public Object
{
    URHO3D_OBJECT(Options, Object);
public:
    Options(Context* context);
    ~Options() = default;

    String loginHost_;
    uint16_t loginPort_;
    String username_;
    String password_;

    void Load();
    void Save();

    int GetWidth() const { return width_; }
    void SetWidth(int value);
    int GetHeight() const { return height_; }
    void SetHeight(int value);
    bool GetFullscreen() const { return fullscreen_; }
    void SetFullscreen(bool value);
    bool GetBorderless() const { return borderless_; }
    void SetBorderless(bool value);
    bool GetResizeable() const { return resizeable_; }
    void SetResizeable(bool value);
    bool GetVSync() const { return vSync_; }
    void SetVSync(bool value);
    bool GetTripleBuffer() const { return tripleBuffer_; }
    void SetTripleBuffer(bool value);
    bool GetHighDPI() const { return highDPI_; }
    void SetHighDPI(bool value);
    void SetMultiSample(int value);
    int GetMultiSample() const { return multiSample_; }
    void SetShadowQuality(ShadowQuality quality);
    ShadowQuality GetShadowQuality() const
    {
        return shadowQuality_;
    }
    /// Set texture quality level. See the QUALITY constants in GraphicsDefs.h.
    void SetTextureQuality(int quality);
    int GetTextureQuality() const
    {
        return textureQuality_;
    }
    /// Set material quality level. See the QUALITY constants in GraphicsDefs.h.
    void SetMaterialQuality(int quality);
    int GetMaterialQuality() const
    {
        return materialQuality_;
    }

    const String& GetRenderPath() const;

    void UpdateAudio();
private:
    int width_;
    int height_;
    bool fullscreen_;
    bool borderless_;
    bool resizeable_;
    bool vSync_;
    bool tripleBuffer_;
    bool highDPI_;
    int multiSample_;
    ShadowQuality shadowQuality_;
    int textureQuality_;
    int materialQuality_;
    // "RenderPaths/Prepass.xml";
    // "RenderPaths/Deferred.xml";
    String renderPath_;

    float gainMaster_;
    float gainEffect_;
    float gainAmbient_;
    float gainVoice_;
    float gainMusic_;

    void UpdateGraphicsMode();
};

