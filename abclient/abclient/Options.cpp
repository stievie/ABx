#include "stdafx.h"
#include "Options.h"

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
    shadowQuality_(SHADOWQUALITY_BLUR_VSM),
    textureQuality_(QUALITY_HIGH),
    materialQuality_(QUALITY_HIGH),
    multiSample_(1),
    loginHost_("localhost"),
    renderPath_("RenderPaths/Prepass.xml"),
    gainMaster_(1.0f),
    gainEffect_(1.0f),
    gainAmbient_(1.0f),
    gainVoice_(1.0f),
    gainMusic_(1.0f),
    stickCameraToHead_(true)
{
}

void Options::Load()
{
    File file(context_, "config.xml");
    SharedPtr<XMLFile> xml(new XMLFile(context_));
    if (!xml->Load(file))
        return;

    XMLElement root = xml->GetRoot();
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

        paramElem = paramElem.GetNext("parameter");
    }
}

void Options::Save()
{
    // TODO
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

void Options::UpdateAudio()
{
    Audio* audio = GetSubsystem<Audio>();
    audio->SetMasterGain(SOUND_MASTER, gainMaster_);
    audio->SetMasterGain(SOUND_EFFECT, gainEffect_);
    audio->SetMasterGain(SOUND_AMBIENT, gainAmbient_);
    audio->SetMasterGain(SOUND_VOICE, gainVoice_);
    audio->SetMasterGain(SOUND_MUSIC, gainMusic_);
}
