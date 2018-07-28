#include "stdafx.h"
#include "PingDot.h"
#include "FwClient.h"
#include <Mustache/mustache.hpp>

#include <Urho3D/DebugNew.h>

const IntRect PingDot::PING_NONE(0, 0, 32, 32);
const IntRect PingDot::PING_GOOD(32, 0, 64, 32);
const IntRect PingDot::PING_OKAY(64, 0, 96, 32);
const IntRect PingDot::PING_BAD(96, 0, 128, 32);

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
    SetTexture(cache->GetResource<Texture2D>("Textures/ping3d.png"));
    SetImageRect(PingDot::PING_NONE);
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
            SetImageRect(PingDot::PING_GOOD);
        else if (lastPing < 300)
            SetImageRect(PingDot::PING_OKAY);
        else
            SetImageRect(PingDot::PING_BAD);

        kainjow::mustache::mustache tmpl{ "FPS: {{fps}}\n\nAverage Ping: {{avg_ping}}ms\nLast Ping: {{last_ping}}ms" };
        kainjow::mustache::data d;
        d.set("fps", std::to_string(fps));
        d.set("avg_ping", std::to_string(c->GetAvgPing()));
        d.set("last_ping", std::to_string(c->GetLastPing()));
        std::string text = tmpl.render(d);
        tooltipText_->SetText(String(text.c_str()));

        lastUpdate_ = 0.0f;
    }
    Button::Update(timeStep);
}
