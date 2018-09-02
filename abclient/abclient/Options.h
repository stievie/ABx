#pragma once

enum class WindowMode
{
    Windowed,
    Fullcreeen,
    Borderless
};

static constexpr float MIN_FOV = 45.0;
static constexpr float MAX_FOV = 120.0;

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

    bool stickCameraToHead_;
    bool disableMouseWalking_;
    float mouseSensitivity_;

    float gainMaster_;
    float gainEffect_;
    float gainAmbient_;
    float gainVoice_;
    float gainMusic_;

    void Load();
    void Save();

    WindowMode GetWindowMode() const
    {
        if (fullscreen_)
            return WindowMode::Fullcreeen;
        if (borderless_)
            return WindowMode::Borderless;
        return WindowMode::Windowed;
    }
    void SetWindowMode(WindowMode mode);
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
    void SetTextureQuality(MaterialQuality quality);
    MaterialQuality GetTextureQuality() const
    {
        return textureQuality_;
    }
    /// Set material quality level. See the QUALITY constants in GraphicsDefs.h.
    void SetMaterialQuality(MaterialQuality quality);
    MaterialQuality GetMaterialQuality() const
    {
        return materialQuality_;
    }
    TextureFilterMode GetTextureFilterMode() const
    {
        return textureFilterMode_;
    }
    void SetTextureFilterMode(TextureFilterMode value);
    int GetTextureAnisotropyLevel() const
    {
        return textureAnisotropyLevel_;
    }
    void SetTextureAnisotropyLevel(int value);
    bool GetShadows() const
    {
        return shadows_;
    }
    void SetShadows(bool value);
    float GetCameraFarClip() const
    {
        return cameraFarClip_;
    }
    float GetCameraNearClip() const
    {
        return cameryNearClip_;
    }
    float GetCameraFov() const
    {
        return cameraFov_;
    }
    void SetCameraFov(float value)
    {
        float fov = Clamp(value, MIN_FOV, MAX_FOV);
        if (fov != cameraFov_)
        {
            cameraFov_ = fov;
        }
    }
    const IntVector2& GetWindowPos() const
    {
        return windowPos_;
    }

    const String& GetRenderPath() const;

    void UpdateAudio();
    void LoadWindow(UIElement* window);
    void SaveWindow(UIElement* window);

    static const String& GetPrefPath();
    static void SetPrefPath(const String& value);
    static bool CreateDir(const String& path);
private:
    static String prefPath_;
    IntVector2 oldWindowPos_;
    IntVector2 windowPos_;
    int width_;
    int height_;
    bool fullscreen_;
    bool borderless_;
    bool resizeable_;
    bool vSync_;
    bool tripleBuffer_;
    bool highDPI_;
    int multiSample_;
    bool shadows_;
    float cameraFarClip_;
    float cameryNearClip_;
    float cameraFov_;
    ShadowQuality shadowQuality_;
    MaterialQuality textureQuality_;
    MaterialQuality materialQuality_;
    TextureFilterMode textureFilterMode_;
    int textureAnisotropyLevel_;
    // "RenderPaths/Prepass.xml";
    // "RenderPaths/Deferred.xml";
    String renderPath_;

    void UpdateGraphicsMode();
    void LoadSettings();
    void LoadElements(const XMLElement& root);
};

