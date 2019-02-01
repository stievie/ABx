#include "stdafx.h"
#include "CreditsWindow.h"

String CreditsWindow::NAME = "CreditsWindow";

void CreditsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<CreditsWindow>();
}

CreditsWindow::CreditsWindow(Context* context) :
    Window(context),
    totalCreditsHeight_(0),
    creditLengthInSeconds_(0)
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
    totalCreditsHeight_ = 0;
    creditLengthInSeconds_ = 0;
    creditsBase_.Reset();
    credits_.Clear();

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

void CreditsWindow::CreateLogo(const String& file, float scale)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Texture2D* logoTexture = cache->GetResource<Texture2D>(file);
    if (!logoTexture)
        return;

    totalCreditsHeight_ += 5;
    // Create logo sprite and add to the UI layout
    SharedPtr<BorderImage> logoSprite(creditsBase_->CreateChild<BorderImage>());

    // Set logo sprite texture
    logoSprite->SetTexture(logoTexture);
    int textureWidth = logoTexture->GetWidth();
    int textureHeight = logoTexture->GetHeight();
    int width = static_cast<int>((float)textureWidth * scale);
    int height = static_cast<int>((float)textureHeight * scale);
    logoSprite->SetFullImageRect();
    logoSprite->SetPosition(IntVector2(0, totalCreditsHeight_));
    logoSprite->SetSize(IntVector2(width, height));
    logoSprite->SetAlignment(HA_CENTER, VA_TOP);
    credits_.Push(logoSprite);
    totalCreditsHeight_ += height;
}

void CreditsWindow::AddCredits()
{
    CreateSingleLine("Creator", 30, true);
    CreateSingleLine("sa", 15);
    CreateLogo("Textures/Trill.png", 0.05f);

    CreateSingleLine("Common", 30, true);
    CreateSingleLine("asio: https://think-async.com/Asio/AsioStandalone", 15);
    CreateSingleLine("OpenSSL: https://www.openssl.org/", 15);
    CreateSingleLine("DH key exchange: https://github.com/thejinchao/dhexchange", 15);
    CreateSingleLine("Catch2: https://github.com/catchorg/Catch2", 15);
    CreateSingleLine("stb: https://github.com/nothings/stb", 15);

    CreateSingleLine("Server", 30, true);
    CreateSingleLine("Recast & Detour: https://github.com/recastnavigation/recastnavigation", 15);
    CreateSingleLine("SimpleAI: https://github.com/mgerhardy/simpleai", 15);
    CreateSingleLine("SimpleWeb: https://github.com/eidheim/Simple-Web-Server", 15);
    CreateSingleLine("Simple cache: https://github.com/brimzi/simple-cache", 15);
    CreateSingleLine("Lua 5.3.4: https://www.lua.org/", 15);
    CreateLogo("Textures/lua-official.png", 0.25f);
    CreateSingleLine("Kaguya: https://github.com/satoren/kaguya", 15);
    CreateSingleLine("pugixml: https://pugixml.org/", 15);
    CreateSingleLine("DirectXMath: https://github.com/Microsoft/DirectXMath", 15);
    CreateSingleLine("XMath: https://github.com/Napoleon314/XMath", 15);
    CreateSingleLine("GJK: https://github.com/xuzebin/gjk", 15);
    CreateSingleLine("Binary serialization: https://github.com/fraillt/bitsery", 15);
    CreateSingleLine("UUIDs: https://github.com/mariusbancila/stduuid", 15);
    CreateSingleLine("boost: https://www.boost.org/", 15);
    CreateSingleLine("ginger Template Engine: https://github.com/melpon/ginger", 15);
    CreateSingleLine("SimpleJSON: https://github.com/nbsdx/SimpleJSON", 15);
    CreateSingleLine("LESS complier: https://github.com/BramvdKroef/clessc", 15);
    CreateSingleLine("Password hash: bcrypt OpenBSD https://www.openbsd.org/", 15);
    CreateSingleLine("PRNG: arc4random OpenBSD https://www.openbsd.org/", 15);
    CreateSingleLine("Database: PostgreSQL https://www.postgresql.org/", 15);
    CreateLogo("Textures/PostgreSQL_logo.png", 0.5f);

    CreateSingleLine("Client", 30, true);
    CreateSingleLine("Urho3D: https://urho3d.github.io/", 15);
    CreateLogo("Textures/FishBoneLogo.png", 0.5f);
    CreateSingleLine("Mustache: https://github.com/kainjow/Mustache", 15);
    CreateSingleLine("PostProcessController: https://gist.github.com/lezak", 15);
    CreateSingleLine("Level manager: https://urho3d.prophpbb.com/topic2367.html", 15);
    CreateSingleLine("Entity Position Interpolation: https://github.com/jwatte/EPIC", 15);
    CreateSingleLine("Ocean Simulation: https://github.com/Lumak/Urho3D-Ocean-Simulation", 15);
    CreateSingleLine("Particles: https://kenney.nl/assets/particle-pack", 15);
    CreateSingleLine("Some code: https://github.com/ArnisLielturks/Urho3D-Empty-Project", 15);

    CreateSingleLine("Assets", 30, true);
    CreateSingleLine("World Map: https://en.wikipedia.org/wiki/File:Map_greek_sanctuaries-en.svg", 15);
    CreateSingleLine("Creative Commons Attribution-Share Alike 3.0 Unported License", 10);
    CreateSingleLine("https://creativecommons.org/licenses/by-sa/3.0/deed.en", 10);
    CreateSingleLine("Fonts: ClearSans, Anonymous Pro (Urho3Ds default font)", 15);

    CreateSingleLine("Music", 25, true);
    CreateSingleLine("Kevin MacLeod (incompetech.com):", 15);
    CreateSingleLine("Licensed under Creative Commons: By Attribution 3.0 License", 10);
    CreateSingleLine("https://creativecommons.org/licenses/by/3.0/", 10);
    CreateSingleLine("For complete list see CREDITS.md", 20);

    CreateSingleLine("Tools", 30, true);
    CreateSingleLine("3D Modeling: Blender https://www.blender.org", 15);
    CreateLogo("Textures/blender_logo.png", 0.8f);
    CreateSingleLine("Human Models: MakeHuman http://www.makehumancommunity.org", 15);
    CreateLogo("Textures/makehuman_logo.png", 0.5f);
    CreateSingleLine("Animations: mixamo https://www.mixamo.com/", 15);
    CreateSingleLine("Sounds: Audacity https://www.audacityteam.org/", 15);
    CreateLogo("Textures/Audacity_Logo.png", 0.5f);
    CreateSingleLine("Inkscape https://inkscape.org/", 15);
    CreateLogo("Textures/inkscape.png", 1.0f);
}
