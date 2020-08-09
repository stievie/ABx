/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#include <Urho3DAll.h>

enum class WindowMode
{
    Windowed,
    Fullcreen,
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

inline constexpr float MIN_FOV = 45.0;
inline constexpr float MAX_FOV = 120.0;
inline constexpr float CAMERA_MIN_DIST = 0.75f;
inline constexpr float CAMERA_INITIAL_DIST = 10.0f;
inline constexpr float CAMERA_MAX_DIST = 40.0f;

struct Environment
{
    String name;
    String host;
    uint16_t port;
    bool selected;
};

class Options : public Object
{
    URHO3D_OBJECT(Options, Object)
public:
    Options(Context* context);
    ~Options() override;

    String loginHost_;
    uint16_t loginPort_{ 0 };
    String username_;
    String password_;
    Vector<Environment> environments_;

    bool stickCameraToHead_{ true };
    bool soundListenerToHead_{ false };
    bool disableMouseWalking_{ false };
    bool enableMumble_{ false };
    float mouseSensitivity_{ 0.1f };

    float gainMaster_{ 1.0f };
    float gainEffect_{ 1.0f };
    float gainAmbient_{ 1.0f };
    float gainVoice_{ 1.0f };
    float gainMusic_{ 0.1f };

    void Load();
    void Save();

    String GetDataFile(const String& file) const;
    Environment* GetEnvironmment(const String& name);
    Environment* GetSelectedEnvironment();
    void SetSelectedEnvironment(const String& name);
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
    bool IsMaximized() const
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
    float GetShadowSoftness() const { return shadowSoftness_; }
    void SetShadowSoftness(float value);
    unsigned GetShadowMapSize() const { return shadowMapSize_; }
    void SetShadowMapSize(unsigned value);
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
        return cameraNearClip_;
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
    unsigned GetChatInputHistorySize() const
    {
        return chatInputHistorySize_;
    }
    void SetChatInputHistorySize(unsigned value)
    {
        chatInputHistorySize_ = value;
    }

    const String& GetRenderPath() const;

    void UpdateAudio();
    /// Toggle
    void MuteAudio();
    void LoadWindow(UIElement* window);
    void SaveWindow(UIElement* window);

    static const String& GetPrefPath();
    static void SetPrefPath(const String& value);
    static bool CreateDir(const String& path);
private:
    static String prefPath_;
    IntVector2 oldWindowPos_;
    IntVector2 windowPos_;
    unsigned chatInputHistorySize_{ 20 };
    int width_{ 0 };
    int height_{ 0 };
    bool fullscreen_{ true };
    bool borderless_{ false };
    bool resizeable_{ false };
    bool maximized_{ false };
    bool internalMaximized_{ false };
    bool vSync_{ true };
    int maxFps_{ 200 };
    bool tripleBuffer_{ false };
    bool highDPI_{ false };
    bool shadows_{ true };
    float cameraFarClip_{ 300.0f };
    float cameraNearClip_{ 0.0f };
    float cameraFov_{ 60.0f };
    ShadowQuality shadowQuality_{ SHADOWQUALITY_VSM };
    MaterialQuality textureQuality_{ QUALITY_HIGH };
    MaterialQuality materialQuality_{ QUALITY_HIGH };
    TextureFilterMode textureFilterMode_{ FILTER_ANISOTROPIC };
    float shadowSoftness_{ 0.5f };
    unsigned shadowMapSize_{ 4096 };
    int textureAnisotropyLevel_{ 16 };
    AntiAliasingMode antiAliasingMode_{ AntiAliasingMode::FXAA3 };
    bool specularLightning_{ true };
    bool hdrRendering_{ false };
    String renderPath_;

    void UpdateGraphicsMode();
    void LoadSettings();
    void LoadElements(const XMLElement& root);
    void HandleInputFocus(StringHash eventType, VariantMap& eventData);
};

