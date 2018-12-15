#include "stdafx.h"
#include "CreditsWindow.h"

String CreditsWindow::NAME = "CreditsWindow";

void CreditsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<CreditsWindow>();
}

CreditsWindow::CreditsWindow(Context* context) :
    Window(context)
{
    SetName(CreditsWindow::NAME);
    auto* graphics = GetSubsystem<Graphics>();
    SetSize(graphics->GetWidth(), graphics->GetHeight());
    SetLayout(LM_FREE);
    // Center this window in it's parent element.
    SetAlignment(HA_CENTER, VA_CENTER);
    // Black color
    SetColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
    SetOpacity(0.9f);
    SetPriority(100);
    // Make it top most
    SetBringToBack(false);
    BringToFront();

    CreateUI();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(CreditsWindow, HandleUpdate));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(CreditsWindow, HandleKeyDown));
}

CreditsWindow::~CreditsWindow()
{
    UnsubscribeFromAllEvents();
}

void CreditsWindow::Close()
{
    UnsubscribeFromEvent(E_UPDATE);
    GetSubsystem<UI>()->GetRoot()->RemoveChild(this);
}

void CreditsWindow::CreateUI()
{
    timer_.Reset();
    creditsBase_ = CreateChild<UIElement>();
    creditsBase_->SetAlignment(HA_CENTER, VA_BOTTOM);
    creditsBase_->SetStyleAuto();

    AddCredits();
    creditLengthInSeconds_ = credits_.Size() * 2;

    auto* graphics = GetSubsystem<Graphics>();
    SharedPtr<ObjectAnimation> animation(new ObjectAnimation(context_));
    SharedPtr<ValueAnimation> colorAnimation(new ValueAnimation(context_));
    // Use spline interpolation method
    colorAnimation->SetInterpolationMethod(IM_SPLINE);
    // Set spline tension
    colorAnimation->SetSplineTension(0.7f);
    colorAnimation->SetKeyFrame(0.0f, IntVector2(0, 0));
    colorAnimation->SetKeyFrame(static_cast<float>(creditLengthInSeconds_),
        IntVector2(0, -graphics->GetHeight() - totalCreditsHeight_ - 50));
    colorAnimation->SetKeyFrame(static_cast<float>(creditLengthInSeconds_ * 2),
        IntVector2(0, -graphics->GetHeight() - totalCreditsHeight_ - 50));
    animation->AddAttributeAnimation("Position", colorAnimation);
    creditsBase_->SetObjectAnimation(animation);
}

void CreditsWindow::HandleUpdate(StringHash, VariantMap&)
{
    if (timer_.GetMSec(false) > static_cast<unsigned>(creditLengthInSeconds_) * 1000u)
    {
        Close();
    }
}

void CreditsWindow::HandleKeyDown(StringHash, VariantMap& eventData)
{
    using namespace KeyDown;
    Key key = static_cast<Key>(eventData[P_KEY].GetInt());
    if (key == KEY_ESCAPE)
        Close();
}

void CreditsWindow::CreateSingleLine(const String& content, int fontSize, bool bold /* = false */)
{
    totalCreditsHeight_ += static_cast<int>(static_cast<float>(fontSize) / 1.5f);

    auto cache = GetSubsystem<ResourceCache>();
    auto* font = bold ?
        cache->GetResource<Font>("Fonts/ClearSans-Bold.ttf") :
        cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf");

    SharedPtr<Text> text(creditsBase_->CreateChild<Text>());
    text->SetPosition(IntVector2(0, totalCreditsHeight_));
    text->SetAlignment(HA_CENTER, VA_TOP);
    text->SetStyleAuto();
    text->SetFont(font, static_cast<float>(fontSize));
    text->SetText(content);
    credits_.Push(text);
    totalCreditsHeight_ += static_cast<int>(static_cast<float>(fontSize) * 1.5f);
}

void CreditsWindow::AddCredits()
{
    CreateSingleLine("Creator", 30, true);
    CreateSingleLine("sa", 15);

    CreateSingleLine("Common", 30, true);
    CreateSingleLine("asio: https://think-async.com/Asio/AsioStandalone", 15);
    CreateSingleLine("OpenSSL: https://www.openssl.org/", 15);
    CreateSingleLine("DH key exchange: https://github.com/thejinchao/dhexchange", 15);

    CreateSingleLine("Server", 30, true);
    CreateSingleLine("Recast & Detour: https://github.com/recastnavigation/recastnavigation", 15);
    CreateSingleLine("SimpleAI: https://github.com/mgerhardy/simpleai", 15);
    CreateSingleLine("SimpleWeb: https://github.com/eidheim/Simple-Web-Server", 15);
    CreateSingleLine("Simple cache: https://github.com/brimzi/simple-cache", 15);
    CreateSingleLine("Lua 5.3.4: https://www.lua.org/", 15);
    CreateSingleLine("Kaguya: https://github.com/satoren/kaguya", 15);
    CreateSingleLine("pugixml: https://pugixml.org/", 15);
    CreateSingleLine("GJK: https://github.com/xuzebin/gjk", 15);
    CreateSingleLine("Binary serialization: https://github.com/fraillt/bitsery", 15);
    CreateSingleLine("UUIDs: https://github.com/mariusbancila/stduuid", 15);
    CreateSingleLine("boost: https://www.boost.org/", 15);
    CreateSingleLine("ginger Template Engine: https://github.com/melpon/ginger", 15);
    CreateSingleLine("SimpleJSON: https://github.com/nbsdx/SimpleJSON", 15);
    CreateSingleLine("LESS complier: https://github.com/BramvdKroef/clessc", 15);

    CreateSingleLine("Client", 30, true);
    CreateSingleLine("Urho3D: https://urho3d.github.io/", 15);
    CreateSingleLine("Mustache: https://github.com/kainjow/Mustache", 15);
    CreateSingleLine("PostProcessController: https://gist.github.com/lezak", 15);
    CreateSingleLine("Level manager: https://urho3d.prophpbb.com/topic2367.html", 15);
    CreateSingleLine("Entity Position Interpolation: https://github.com/jwatte/EPIC", 15);
    CreateSingleLine("Particles: http://kenney.nl/assets/particle-pack", 15);
    CreateSingleLine("Urho3D-Empty-Project: https://github.com/ArnisLielturks/Urho3D-Empty-Project", 15);

    CreateSingleLine("Assets", 30, true);
    CreateSingleLine("Human Models: MakeHuman http://www.makehumancommunity.org", 15);
    CreateSingleLine("Animations: mixamo https://www.mixamo.com/", 15);
    CreateSingleLine("World Map: https://en.wikipedia.org/wiki/File:Map_greek_sanctuaries-en.svg", 15);
    CreateSingleLine("Creative Commons Attribution-Share Alike 3.0 Unported License", 10);
    CreateSingleLine("https://creativecommons.org/licenses/by-sa/3.0/deed.en", 10);
    CreateSingleLine("Fonts: ClearSans, Anonymous Pro (Urho3Ds default font)", 15);

    CreateSingleLine("Music", 25, true);
    CreateSingleLine("Mostly by Kevin MacLeod (incompetech.com):", 15);
    CreateSingleLine("\"Virtutes Instrumenti\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Virtutes Vocis\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Relent\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Return of the Mummy\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"The Pyre\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Chee Zee Caves V2\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Mystery Bazaar\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Hidden Wonders\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Naraina\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Lord of the Land\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Teller of the Tales\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Jalandhar\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Tabuk\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Ibn Al-Noor\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Chee Zee Jungle\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Desert City\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"String Impromptu Number 1\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Agnus Dei X\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Ghost Dance\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Twisting\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Hiding Your Reality\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Outfoxing the Fox\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Hot Pursuit\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"I Can Feel it Coming\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Aggressor\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Ghost Dance\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Unholy Knight\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Burnt Spirit\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Killers\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Rynos Theme\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("\"Curse of the Scarab\" Kevin MacLeod (incompetech.com)", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("http://creativecommons.org/licenses/by/3.0/", 10);

    CreateSingleLine("", 15);
    CreateSingleLine("See CREDITS.md", 20);
}
