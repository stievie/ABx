#include "stdafx.h"
#include "PingDot.h"
#include "FwClient.h"

#include <Urho3D/DebugNew.h>

void PingDot::RegisterObject(Context* context)
{
    context->RegisterFactory<PingDot>();
}

PingDot::PingDot(Context* context) :
    Button(context),
    lastUpdate_(0.0f)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/PingTooltip.xml");
    LoadChildXML(chatFile->GetRoot());
    tooltipText_ = dynamic_cast<Text*>(GetChild("TooltipText", true));
    SetTexture(cache->GetResource<Texture2D>("Textures/ping_none.png"));
}

PingDot::~PingDot()
{
    UnsubscribeFromAllEvents();
}

void PingDot::Update(float timeStep)
{
    lastUpdate_ += timeStep;
    if (lastUpdate_ >= 1.0f)
    {
        FwClient* c = context_->GetSubsystem<FwClient>();
        Time* t = context_->GetSubsystem<Time>();
        int fps = static_cast<int>(1.0f / t->GetTimeStep());
        int lastPing = c->GetLastPing();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Texture2D* pingTexture = nullptr;
        if (lastPing < 110)
            pingTexture = cache->GetResource<Texture2D>("Textures/ping_good.png");
        else if (lastPing < 300)
            pingTexture = cache->GetResource<Texture2D>("Textures/ping_okish.png");
        else
            pingTexture = cache->GetResource<Texture2D>("Textures/ping_bad.png");
        SetTexture(pingTexture);
        std::stringstream s;
        s << "FPS: " << fps << "\n\n";
        s << "Average Ping: " << c->GetAvgPing() << "ms\nLast Ping: " << c->GetLastPing() << "ms";
        tooltipText_->SetText(String(s.str().c_str()));

        lastUpdate_ = 0.0f;
    }
    BorderImage::Update(timeStep);
}
