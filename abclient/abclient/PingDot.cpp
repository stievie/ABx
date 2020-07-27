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

#include "PingDot.h"
#include "FwClient.h"
#include "Conversions.h"
#include <sa/Compiler.h>
#include <sa/TemplateParser.h>

//#include <Urho3D/DebugNew.h>

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
    SetName("PingDot");
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/PingTooltip.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);
    tooltipText_ = GetChildStaticCast<Text>("TooltipText", true);
    auto tex = cache->GetResource<Texture2D>("Textures/PingDot.png");
    SetTexture(tex);
    SetImageRect(PingDot::PING_NONE);

    SetSize(IntVector2(16, 16));
    SetAlignment(HA_RIGHT, VA_BOTTOM);
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
        FwClient* c = GetSubsystem<FwClient>();
        Time* t = GetSubsystem<Time>();
        int fps = static_cast<int>(1.0f / t->GetTimeStep());
        int lastPing = c->GetLastPing();
        if (lastPing < 110)
            SetImageRect(PingDot::PING_GOOD);
        else if (lastPing < 300)
            SetImageRect(PingDot::PING_OKAY);
        else
            SetImageRect(PingDot::PING_BAD);

        static sa::Template tokens;
        if (tokens.IsEmpty())
        {
            static constexpr const char* TEMPLATE = "FPS: ${fps}\n\nAverage Ping: ${avg_ping}ms\nLast Ping: ${last_ping}ms";
            sa::TemplateParser parser;
            tokens = parser.Parse(TEMPLATE);
        }
        const std::string text = tokens.ToString([&](const sa::Token& token) -> std::string
        {
            switch (token.type)
            {
            case sa::Token::Type::Expression:
                if (token.value == "fps")
                    return std::to_string(fps);
                if (token.value == "avg_ping")
                    return std::to_string(c->GetAvgPing());
                if (token.value == "last_ping")
                    return std::to_string(c->GetLastPing());
                ASSERT_FALSE();
            default:
                return token.value;
            }
        });
        tooltipText_->SetText(ToUrhoString(text));

        lastUpdate_ = 0.0f;
    }
    Button::Update(timeStep);
}
