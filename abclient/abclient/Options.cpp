#include "stdafx.h"
#include "Options.h"
#include <SDL/SDL_filesystem.h>

#include <Urho3D/DebugNew.h>

Options::Options(Context* context) :
    Object(context),
    width_(0),
    height_(0),
    fullscreen_(true),
    borderless_(false),
    resizeable_(false),
    highDPI_(false),
    vSync_(false),
    tripleBuffer_(false),
    cameraFarClip_(300.0f),
    cameryNearClip_(0.0f),
    cameraFov_(60.0f),
    shadows_(true),
    shadowQuality_(SHADOWQUALITY_BLUR_VSM),
    textureQuality_(QUALITY_HIGH),
    materialQuality_(QUALITY_HIGH),
    textureFilterMode_(FILTER_ANISOTROPIC),
    textureAnisotropyLevel_(16),
    multiSample_(1),
    loginHost_("localhost"),
    renderPath_("RenderPaths/Prepass.xml"),
    gainMaster_(1.0f),
    gainEffect_(1.0f),
    gainAmbient_(1.0f),
    gainVoice_(1.0f),
    gainMusic_(1.0f),
    stickCameraToHead_(true),
    disableMouseWalking_(false)
{
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
    // TODO
    String prefPath = GetPrefPath();
#ifdef _WIN32
    CreateDirectoryW(WString(prefPath).CString(), nullptr);
#endif
    SharedPtr<XMLFile> xml(new XMLFile(context_));

    XMLElement root = xml->CreateRoot("settings");
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowWidth");
        param.SetString("type", "int");
        param.SetInt("value", width_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "WindowHeight");
        param.SetString("type", "int");
        param.SetInt("value", height_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Fullscreen");
        param.SetString("type", "bool");
        param.SetBool("value", fullscreen_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "Borderless");
        param.SetString("type", "bool");
        param.SetBool("value", borderless_);
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
        param.SetInt("value", shadows_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "MultiSample");
        param.SetString("type", "int");
        param.SetInt("value", multiSample_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "MultiSample");
        param.SetString("type", "int");
        param.SetInt("value", multiSample_);
    }
    {
        XMLElement param = root.CreateChild("parameter");
        param.SetString("name", "RenderPath");
        param.SetString("type", "string");
        param.SetString("value", renderPath_);
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

    xml->SaveFile(AddTrailingSlash(prefPath) + "settings.xml");
}

void Options::SetMultiSample(int value)
{
    if (multiSample_ != value)
    {
        multiSample_ = value;
        UpdateGraphicsMode();
    }
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
        shadows_ = false;
        GetSubsystem<Renderer>()->SetDrawShadows(shadows_);
    }
}

const String& Options::GetRenderPath() const
{
    return renderPath_;
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

void Options::SetFullscreen(bool value)
{
    if (fullscreen_ != value)
    {
        fullscreen_ = value;
        UpdateGraphicsMode();
    }
}

void Options::SetBorderless(bool value)
{
    if (borderless_ != value)
    {
        borderless_ = value;
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
    graphics->SetMode(width_, height_, fullscreen_, borderless_, resizeable_,
        highDPI_, vSync_, tripleBuffer_, multiSample_, 0, 0);
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
        else if (name.Compare("HighDPI") == 0)
        {
            highDPI_ = paramElem.GetBool("value");
        }
        else if (name.Compare("VSync") == 0)
        {
            vSync_ = paramElem.GetBool("value");
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
        else if (name.Compare("MultiSample") == 0)
        {
            multiSample_ = paramElem.GetInt("value");
        }
        else if (name.Compare("LoginPort") == 0)
        {
            loginPort_ = paramElem.GetInt("value");
        }
        else if (name.Compare("LoginHost") == 0)
        {
            loginHost_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("Username") == 0)
        {
            username_ = paramElem.GetAttribute("value");
        }
        else if (name.Compare("Password") == 0)
        {
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

        paramElem = paramElem.GetNext("parameter");
    }
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

String Options::GetPrefPath()
{
    char* pathName = SDL_GetPrefPath("Trill", "FW");
    if (pathName)
    {
        String ret(pathName);
        SDL_free(pathName);
        return ret;
    }
    return String();
}
