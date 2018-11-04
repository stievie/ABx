#pragma once

enum class WindowMode
{
    Windowed,
    Fullcreeen,
    Borderless,
    Maximized
};

enum class AntiAliasingMode
{
    None,
    FXAA3,             /// Fast approx. AA
    MSAAx2,            /// 2x multi sample
    MSAAx4,
    MSAAx8,
    MSAAx16
};

static constexpr float MIN_FOV = 45.0;
static constexpr float MAX_FOV = 120.0;

class Options : public Object
{
    URHO3D_OBJECT(Options, Object);
public:
    Options(Context* context);
    ~Options();

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

    WindowMode GetWindowMode() const;
    void SetWindowMode(WindowMode mode);
    int GetWidth() const
    {
        return width_;
    }
    void SetWidth(int value);
    int GetHeight() const
    {
        return height_;
    }
    void SetHeight(int value);
    bool GetFullscreen() const
    {
        return fullscreen_;
    }
    void SetFullscreen(bool value);
    bool GetBorderless() const
    {
        return borderless_;
    }
    void SetBorderless(bool value);
    bool GetResizeable() const
    {
        return resizeable_;
    }
    void SetResizeable(bool value);
    bool IsMiximized() const
    {
        return maximized_;
    }
    bool GetVSync() const
    {
        return vSync_;
    }
    void SetVSync(bool value);
    int GetMaxFps() const
    {
        return maxFps_ > 0 ? maxFps_ : 200;
    }
    void SetMaxFps(int value);
    bool GetTripleBuffer() const
    {
        return tripleBuffer_;
    }
    void SetTripleBuffer(bool value);
    bool GetHighDPI() const
    {
        return highDPI_;
    }
    void SetHighDPI(bool value);
    int GetMultiSample() const
    {
        switch (antiAliasingMode_)
        {
        case AntiAliasingMode::MSAAx2:
            return 2;
        case AntiAliasingMode::MSAAx4:
            return 4;
        case AntiAliasingMode::MSAAx8:
            return 8;
        case AntiAliasingMode::MSAAx16:
            return 16;
        default:
            return 1;
        }
    }
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
    bool GetSpecularLightning() const
    {
        return specularLightning_;
    }
    void SetSpecularLightning(bool value);
    bool GetHDRRendering() const
    {
        return hdrRendering_;
    }
    void SetHDRRendering(bool value);
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
    void SetCameraFov(float value);
    AntiAliasingMode GetAntiAliasingMode() const
    {
        return antiAliasingMode_;
    }
    void SetAntiAliasingMode(AntiAliasingMode mode);
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
    bool maximized_;
    bool internalMaximized_;
    bool vSync_;
    int maxFps_;
    bool tripleBuffer_;
    bool highDPI_;
    bool shadows_;
    float cameraFarClip_;
    float cameryNearClip_;
    float cameraFov_;
    ShadowQuality shadowQuality_;
    MaterialQuality textureQuality_;
    MaterialQuality materialQuality_;
    TextureFilterMode textureFilterMode_;
    AntiAliasingMode antiAliasingMode_;
    int textureAnisotropyLevel_;
    bool specularLightning_;
    bool hdrRendering_;
    // "RenderPaths/Prepass.xml";
    // "RenderPaths/Deferred.xml";
    String renderPath_;

    void UpdateGraphicsMode();
    void LoadSettings();
    void LoadElements(const XMLElement& root);
    void HandleInputFocus(StringHash eventType, VariantMap& eventData);
};

