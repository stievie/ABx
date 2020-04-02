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

#include "stdafx.h"
#include "Options.h"
#include <SDL/SDL_filesystem.h>
#include "Shortcuts.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#if __cplusplus < 201703L
#   if !defined(__clang__)
#       include <filesystem>
#   else
#       include <experimental/filesystem>
#   endif
#else
#   include <filesystem>
#endif

//#include <Urho3D/DebugNew.h>

String Options::prefPath_;

Options::Options(Context* context) :
    Object(context),
    loginHost_("localhost"),
    renderPath_("RenderPaths/Prepass.xml")
{
    SubscribeToEvent(E_INPUTFOCUS, URHO3D_HANDLER(Options, HandleInputFocus));
}

Options::~Options()
{
    UnsubscribeFromAllEvents();
}

void Options::Load()
{
    File file(context_, "config.xml");
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(file))
        return;

    XMLElement root = xml->GetRoot();
    LoadElements(root);

    // Settings override default options
    LoadSettings();
}

void Options::Save()
{
    String prefPath = GetPrefPath();
    if (!CreateDir(prefPath))
    {
        URHO3D_LOGERRORF("Failed to create directory %s", prefPath.CString());
        return;
    }
    String file = AddTrailingSlash(prefPath) + "settings.xml";
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    xml->LoadFile(file);

    XMLElement root = xml->GetRoot();
    if (!root)
        root = xml->CreateRoot("settings");
    root.RemoveChildren("parameter");

    maximized_ = internalMaximized_;
    Graphics* graphics = GetSubsystem<Graphics>();
    WindowMode windowMode = GetWindowMode();

    IntVector2 windowPos = graphics->GetWindowPosition();
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowWidth");
        param.SetString("type", "int");
        param.SetInt("value", windowMode == WindowMode::Windowed ? graphics->GetWidth() : width_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowHeight");
        param.SetString("type", "int");
        param.SetInt("value", windowMode == WindowMode::Windowed ? graphics->GetHeight() : height_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowPosX");
        param.SetString("type", "int");
        param.SetInt("value", windowMode == WindowMode::Windowed ? windowPos.x_ : windowPos_.x_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowPosY");
        param.SetString("type", "int");
        param.SetInt("value", windowMode == WindowMode::Windowed ? windowPos.y_ : windowPos_.y_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Fullscreen");
        param.SetString("type", "bool");
        param.SetBool("value", windowMode == WindowMode::Fullcreen);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Borderless");
        param.SetString("type", "bool");
        param.SetBool("value", windowMode == WindowMode::Borderless);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Maximized");
        param.SetString("type", "bool");
        param.SetBool("value", windowMode == WindowMode::Maximized);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Resizeable");
        param.SetString("type", "bool");
        param.SetBool("value", resizeable_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "HighDPI");
        param.SetString("type", "bool");
        param.SetBool("value", highDPI_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "VSync");
        param.SetString("type", "bool");
        param.SetBool("value", vSync_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "MaxFps");
        param.SetString("type", "int");
        param.SetInt("value", maxFps_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "TripleBuffer");
        param.SetString("type", "bool");
        param.SetBool("value", tripleBuffer_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "ShadowQuality");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(shadowQuality_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "SpecularLightning");
        param.SetString("type", "bool");
        param.SetBool("value", specularLightning_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "HDRRendering");
        param.SetString("type", "bool");
        param.SetBool("value", hdrRendering_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "TextureQuality");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(textureQuality_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "MaterialQuality");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(materialQuality_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "TextureFilterMode");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(textureFilterMode_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "TextureAnisotropy");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(textureAnisotropyLevel_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Shadows");
        param.SetString("type", "bool");
        param.SetBool("value", shadows_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "AntiAliasing");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(antiAliasingMode_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "RenderPath");
        param.SetString("type", "string");
        param.SetString("value", renderPath_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "FOV");
        param.SetString("type", "int");
        param.SetInt("value", static_cast<int>(cameraFov_));
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "MouseSensitivity");
        param.SetString("type", "float");
        param.SetFloat("value", mouseSensitivity_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "EnableMumble");
        param.SetString("type", "bool");
        param.SetBool("value", enableMumble_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "GainMaster");
        param.SetString("type", "float");
        param.SetFloat("value", gainMaster_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "GainEffect");
        param.SetString("type", "float");
        param.SetFloat("value", gainEffect_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "GainAmbient");
        param.SetString("type", "float");
        param.SetFloat("value", gainAmbient_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "GainVoice");
        param.SetString("type", "float");
        param.SetFloat("value", gainVoice_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "GainMusic");
        param.SetString("type", "float");
        param.SetFloat("value", gainMusic_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "StickCameraToHead");
        param.SetString("type", "bool");
        param.SetBool("value", stickCameraToHead_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "DisableMouseWalking");
        param.SetString("type", "bool");
        param.SetBool("value", disableMouseWalking_);
    }
    {
        Environment* sel = GetSelectedEnvironment();
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "ActiveEnvironment");
        param.SetString("type", "string");
        param.SetString("value", (sel != nullptr ? sel->name : ""));
    }

    Shortcuts* sc = GetSubsystem<Shortcuts>();
    if (sc)
        sc->Save(root);

    xml->SaveFile(file);
}

void Options::SetShadowQuality(ShadowQuality quality)
{
    if (shadowQuality_ != quality)
    {
        shadowQuality_ = quality;
        GetSubsystem<Renderer>()->SetShadowQuality(shadowQuality_);
    }
}

void Options::SetTextureQuality(MaterialQuality quality)
{
    if (textureQuality_ != quality)
    {
        textureQuality_ = quality;
        GetSubsystem<Renderer>()->SetTextureQuality(textureQuality_);
    }
}

void Options::SetMaterialQuality(MaterialQuality quality)
{
    if (materialQuality_ != quality)
    {
        materialQuality_ = quality;
        GetSubsystem<Renderer>()->SetMaterialQuality(materialQuality_);
    }
}

void Options::SetTextureFilterMode(TextureFilterMode value)
{
    if (textureFilterMode_ != value)
    {
        textureFilterMode_ = value;
        GetSubsystem<Renderer>()->SetTextureFilterMode(textureFilterMode_);
    }
}

void Options::SetSpecularLightning(bool value)
{
    if (value != specularLightning_)
    {
        specularLightning_ = value;
        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetSpecularLighting(value);
    }
}

void Options::SetHDRRendering(bool value)
{
    if (value != hdrRendering_)
    {
        hdrRendering_ = value;
        Renderer* renderer = GetSubsystem<Renderer>();
        renderer->SetHDRRendering(value);
    }
}

void Options::SetTextureAnisotropyLevel(int value)
{
    if (textureAnisotropyLevel_ != value)
    {
        textureAnisotropyLevel_ = value;
        GetSubsystem<Renderer>()->SetTextureAnisotropy(textureAnisotropyLevel_);
    }
}

void Options::SetShadows(bool value)
{
    if (shadows_ != value)
    {
        shadows_ = value;
        GetSubsystem<Renderer>()->SetDrawShadows(shadows_);
    }
}

void Options::SetCameraFov(float value)
{
    float fov = Clamp(value, MIN_FOV, MAX_FOV);
    if (fov != cameraFov_)
    {
        cameraFov_ = fov;
        LevelManager* lm = GetSubsystem<LevelManager>();
        Camera* cam = lm->GetCamera();
        if (cam)
            cam->SetFov(cameraFov_);
    }
}

void Options::SetAntiAliasingMode(AntiAliasingMode mode)
{
    if (antiAliasingMode_ != mode)
    {
        antiAliasingMode_ = mode;
        UpdateGraphicsMode();
        LevelManager* lm = GetSubsystem<LevelManager>();
        BaseLevel* lvl = lm->GetCurrentLevel<BaseLevel>();
        auto pp = lvl->GetPostProcessController();
        if (pp)
            pp->SetUseFXAA3(antiAliasingMode_ == AntiAliasingMode::FXAA3);
    }
}

const String& Options::GetRenderPath() const
{
    return renderPath_;
}

Environment* Options::GetEnvironmment(const String& name)
{
    for (auto& env : environments_)
    {
        if (env.name.Compare(name) == 0)
            return &env;
    }
    return nullptr;
}

Environment* Options::GetSelectedEnvironment()
{
    for (auto& env : environments_)
    {
        if (env.selected)
            return &env;
    }
    return nullptr;
}

void Options::SetSelectedEnvironment(const String& name)
{
    for (auto& env : environments_)
    {
        env.selected = (env.name.Compare(name) == 0);
    }
}

WindowMode Options::GetWindowMode() const
{
    if (fullscreen_)
        return WindowMode::Fullcreen;
    if (borderless_)
        return WindowMode::Borderless;
    if (internalMaximized_ || maximized_)
        return WindowMode::Maximized;
    return WindowMode::Windowed;
}

void Options::SetWindowMode(WindowMode mode)
{
    if (mode != GetWindowMode())
    {
        if (mode == WindowMode::Fullcreen)
            SetFullscreen(true);
        else if (mode == WindowMode::Borderless)
            SetBorderless(true);
        else
        {
            SetFullscreen(false);
            SetBorderless(false);
        }
    }
}

void Options::SetWidth(int value)
{
    if (width_ != value)
    {
        width_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetHeight(int value)
{
    if (height_ != value)
    {
        height_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetVSync(bool value)
{
    if (vSync_ != value)
    {
        vSync_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetMaxFps(int value)
{
    if (maxFps_ != value)
    {
        Engine* engine = context_->GetSubsystem<Engine>();
        engine->SetMaxFps(value);
        maxFps_ = value;
    }
}

void Options::SetFullscreen(bool value)
{
    if (fullscreen_ != value)
    {
        fullscreen_ = value;
        if (fullscreen_)
            borderless_ = false;
        UpdateGraphicsMode();
    }
}

void Options::SetBorderless(bool value)
{
    if (borderless_ != value)
    {
        borderless_ = value;
        if (borderless_)
            fullscreen_ = false;
        UpdateGraphicsMode();
    }
}

void Options::SetResizeable(bool value)
{
    if (resizeable_ != value)
    {
        resizeable_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetTripleBuffer(bool value)
{
    if (tripleBuffer_ != value)
    {
        tripleBuffer_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetHighDPI(bool value)
{
    if (highDPI_ != value)
    {
        highDPI_ = value;
        UpdateGraphicsMode();
    }
}

void Options::UpdateGraphicsMode()
{
    Graphics* graphics = GetSubsystem<Graphics>();
    bool bl = graphics->GetBorderless();
    int width = 0;
    int height = 0;
    // When switching to borderless automatically size the window
    if (!borderless_ && !fullscreen_)
    {
        height = graphics->GetHeight();
        width = graphics->GetWidth();
    }
    bool blChange = bl != borderless_;
    if (blChange)
    {
        if (borderless_)
            // Store old window position
            oldWindowPos_ = graphics->GetWindowPosition();
    }
    graphics->SetMode(width, height, fullscreen_, borderless_, resizeable_,
        highDPI_, vSync_, tripleBuffer_, GetMultiSample(), 0, 0);
    if (blChange)
    {
        if (GetWindowMode() == WindowMode::Windowed)
            // Switching windowed set old position
            graphics->SetWindowPosition(oldWindowPos_);
        else if (GetWindowMode() == WindowMode::Borderless)
            graphics->SetWindowPosition(0, 0);
    }
}

void Options::LoadSettings()
{
    String prefPath = AddTrailingSlash(GetPrefPath());
    File file(context_, prefPath + "settings.xml");
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(file))
        return;

    XMLElement root = xml->GetRoot();
    LoadElements(root);

    Shortcuts* sc = GetSubsystem<Shortcuts>();
    if (sc)
        sc->Load(root);
}

void Options::LoadElements(const XMLElement& root)
{
    XMLElement paramElem = root.GetChild("parameter");
    while (paramElem)
    {
        String name = paramElem.GetAttribute("name");

        if (name.Compare("WindowWidth") == 0)
        {
            width_ = paramElem.GetInt("value");
        }
        else if (name.Compare("WindowHeight") == 0)
        {
            height_ = paramElem.GetInt("value");
        }
        else if (name.Compare("WindowPosX") == 0)
        {
            windowPos_.x_ = paramElem.GetInt("value");
        }
        else if (name.Compare("WindowPosY") == 0)
        {
            windowPos_.y_ = paramElem.GetInt("value");
        }
        else if (name.Compare("Fullscreen") == 0)
        {
            fullscreen_ = paramElem.GetBool("value");
        }
        else if (name.Compare("Borderless") == 0)
        {
            borderless_ = paramElem.GetBool("value");
        }
        else if (name.Compare("Resizeable") == 0)
        {
            resizeable_ = paramElem.GetBool("value");
        }
        else if (name.Compare("Maximized") == 0)
        {
            maximized_ = paramElem.GetBool("value");
        }
        else if (name.Compare("HighDPI") == 0)
        {
            highDPI_ = paramElem.GetBool("value");
        }
        else if (name.Compare("VSync") == 0)
        {
            vSync_ = paramElem.GetBool("value");
        }
        else if (name.Compare("MaxFps") == 0)
        {
            maxFps_ = paramElem.GetInt("value");
            if (maxFps_ > 200)
                maxFps_ = 200;
        }
        else if (name.Compare("SpecularLightning") == 0)
        {
            specularLightning_ = paramElem.GetBool("value");
        }
        else if (name.Compare("HDRRendering") == 0)
        {
            hdrRendering_ = paramElem.GetBool("value");
        }
        else if (name.Compare("TripleBuffer") == 0)
        {
            tripleBuffer_ = paramElem.GetBool("value");
        }
        else if (name.Compare("ShadowQuality") == 0)
        {
            shadowQuality_ = static_cast<ShadowQuality>(paramElem.GetUInt("value"));
        }
        else if (name.Compare("TextureQuality") == 0)
        {
            textureQuality_ = static_cast<MaterialQuality>(paramElem.GetInt("value"));
        }
        else if (name.Compare("MaterialQuality") == 0)
        {
            materialQuality_ = static_cast<MaterialQuality>(paramElem.GetInt("value"));
        }
        else if (name.Compare("TextureFilterMode") == 0)
        {
            textureFilterMode_ = static_cast<TextureFilterMode>(paramElem.GetInt("value"));
        }
        else if (name.Compare("TextureAnisotropy") == 0)
        {
            textureAnisotropyLevel_ = paramElem.GetInt("value");
        }
        else if (name.Compare("Shadows") == 0)
        {
            shadows_ = paramElem.GetBool("value");
        }
        else if (name.Compare("AntiAliasing") == 0)
        {
            antiAliasingMode_ = static_cast<AntiAliasingMode>(paramElem.GetInt("value"));
        }
        else if (name.Compare("FOV") == 0)
        {
            cameraFov_ = static_cast<float>(paramElem.GetInt("value"));
        }
        else if (name.Compare("MouseSensitivity") == 0)
        {
            mouseSensitivity_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("EnableMumble") == 0)
        {
            enableMumble_ = paramElem.GetBool("value");
        }
        else if (name.Compare("LoginPort") == 0)
        {
            loginPort_ = static_cast<uint16_t>(paramElem.GetUInt("value"));
        }
        else if (name.Compare("LoginHost") == 0)
        {
            loginHost_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("Username") == 0)
        {
            if (username_.Empty())
                username_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("Password") == 0)
        {
            if (password_.Empty())
                password_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("RenderPath") == 0)
        {
            renderPath_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("GainMaster") == 0)
        {
            gainMaster_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("GainEffect") == 0)
        {
            gainEffect_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("GainAmbient") == 0)
        {
            gainAmbient_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("GainVoice") == 0)
        {
            gainVoice_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("GainMusic") == 0)
        {
            gainMusic_ = paramElem.GetFloat("value");
        }
        else if (name.Compare("StickCameraToHead") == 0)
        {
            stickCameraToHead_ = paramElem.GetBool("value");
        }
        else if (name.Compare("DisableMouseWalking") == 0)
        {
            disableMouseWalking_ = paramElem.GetBool("value");
        }
        else if (name.Compare("Environment") == 0)
        {
            Environment env;
            XMLElement nameElem = paramElem.GetChild("name");
            env.name = nameElem.GetAttribute("value");
            XMLElement hostElem = paramElem.GetChild("host");
            env.host = hostElem.GetAttribute("value");
            XMLElement portElem = paramElem.GetChild("port");
            env.port = static_cast<uint16_t>(portElem.GetUInt("value"));
            environments_.Push(env);
        }
        else if (name.Compare("ActiveEnvironment") == 0)
        {
            SetSelectedEnvironment(paramElem.GetAttribute("value"));
        }
        paramElem = paramElem.GetNext("parameter");
    }
}

void Options::HandleInputFocus(StringHash, VariantMap&)
{
    if (GetSubsystem<Graphics>()->GetWindow())
        internalMaximized_ = GetSubsystem<Graphics>()->GetMaximized();
}

void Options::UpdateAudio()
{
    Audio* audio = GetSubsystem<Audio>();
    audio->SetMasterGain(SOUND_MASTER, gainMaster_);
    audio->SetMasterGain(SOUND_EFFECT, gainEffect_);
    audio->SetMasterGain(SOUND_AMBIENT, gainAmbient_);
    audio->SetMasterGain(SOUND_VOICE, gainVoice_);
    audio->SetMasterGain(SOUND_MUSIC, gainMusic_);
}

void Options::MuteAudio()
{
    Audio* audio = GetSubsystem<Audio>();
    if (audio->GetMasterGain(SOUND_MASTER) == gainMaster_)
        audio->SetMasterGain(SOUND_MASTER, 0.0f);
    else
        audio->SetMasterGain(SOUND_MASTER, gainMaster_);
}

void Options::LoadWindow(UIElement* window)
{
    String prefPath = AddTrailingSlash(GetPrefPath());
    File file(context_, prefPath + "settings.xml");
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(file))
        return;

    XMLElement root = xml->GetRoot();
    if (!root)
        return;
    XMLElement winNode = root.GetChild("windows");
    if (!winNode)
        return;
    XMLElement elem = winNode.GetChild(window->GetName());
    if (!elem)
        return;

    window->SetPosition(elem.GetIntVector2("position"));
    if (auto* wnd = dynamic_cast<Window*>(window))
    {
        if (wnd->IsResizable())
            wnd->SetSize(elem.GetIntVector2("size"));
    }
    else
        window->SetSize(elem.GetIntVector2("size"));
    window->SetVisible(elem.GetBool("visible"));
}

void Options::SaveWindow(UIElement* window)
{
    String prefPath = AddTrailingSlash(GetPrefPath());
    String fileName = prefPath + "settings.xml";
    File file(context_, fileName);
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(file))
        return;

    XMLElement root = xml->GetRoot();
    XMLElement winNode = root.HasChild("windows") ? root.GetChild("windows") : root.CreateChild("windows");
    winNode.RemoveChild(window->GetName());
    XMLElement param = winNode.CreateChild(window->GetName());
    param.SetIntVector2("position", window->GetPosition());
    if (auto* wnd = dynamic_cast<Window*>(window))
    {
        if (wnd->IsResizable())
            param.SetIntVector2("size", wnd->GetSize());
    }
    else
        param.SetIntVector2("size", window->GetSize());
    param.SetBool("visible", window->IsVisible());

    xml->SaveFile(fileName);
}

const String& Options::GetPrefPath()
{
    if (prefPath_.Empty())
    {
        char* pathName = SDL_GetPrefPath("Trill", "FW");
        if (pathName)
        {
            prefPath_ = String(pathName);
            SDL_free(pathName);
        }
    }
    return prefPath_;
}

void Options::SetPrefPath(const String& value)
{
    prefPath_ = value;
}

bool Options::CreateDir(const String& path)
{
#if __cplusplus < 201703L
    // C++14
    namespace fs = std::experimental::filesystem;
#else
    // C++17
    namespace fs = std::filesystem;
#endif
    fs::path p(std::string(path.CString()));
    if (fs::exists(p))
        return true;
    return fs::create_directories(p);
}

String Options::GetDataFile(const String& file) const
{
    String result = GetPrefPath();
    if (result.Back() != '/' && result.Back() != '\\' &&
        file.Front() != '/' && file.Front() != '\\')
        result += "/";
    return result + file;
}

std::string Options::GetDataFileStl(const std::string& file) const
{
    return std::string(GetDataFile(String(file.c_str())).CString());
}
