#include "stdafx.h"
#include "PingDot.h"
#include "FwClient.h"

void PingDot::RegisterObject(Context* context)
{
    context->RegisterFactory<PingDot>();
}

PingDot::PingDot(Context* context) :
    Button(context),
    lastUpdate_(0.0f)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml");
    SetDefaultStyle(style);
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/PingTooltip.xml");
    LoadChildXML(chatFile->GetRoot());
    tooltipText_ = dynamic_cast<Text*>(GetChild("TooltipText", true));
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
        s << "Avg. Ping " << c->GetAvgPing() << " Last Ping " << c->GetLastPing();
        tooltipText_->SetText(String(s.str().c_str()));

        lastUpdate_ = 0.0f;
    }
    BorderImage::Update(timeStep);
}
