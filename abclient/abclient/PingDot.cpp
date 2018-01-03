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
    SetTexture(cache->GetResource<Texture2D>("Textures/ping.png"));
    SetImageRect(IntRect(0, 0, 32, 32));
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
        if (lastPing < 110)
            SetImageRect(IntRect(32, 0, 64, 32));
        else if (lastPing < 300)
            SetImageRect(IntRect(64, 0, 96, 32));
        else
            SetImageRect(IntRect(96, 0, 128, 32));

        std::stringstream s;
        s << "FPS: " << fps << "\n\n";
        s << "Average Ping: " << c->GetAvgPing() << "ms\nLast Ping: " << c->GetLastPing() << "ms";
        tooltipText_->SetText(String(s.str().c_str()));

        lastUpdate_ = 0.0f;
    }
    BorderImage::Update(timeStep);
}
