#include "stdafx.h"
#include "Options.h"

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
    multiSample_(1),
    loginHost_("localhost")
{
}

Options::~Options()
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

        paramElem = paramElem.GetNext("parameter");
    }
}

void Options::SetMultiSample(int value)
{
    if (multiSample_ != value)
    {
        multiSample_ = value;
        UpdateGraphicsMode();
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