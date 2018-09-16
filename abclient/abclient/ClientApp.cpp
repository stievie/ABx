#include "stdafx.h"

#include "ClientApp.h"
#include "AbEvents.h"
#include "LoginLevel.h"
#include "OutpostLevel.h"
#include "PvpCombatLevel.h"
#include "CharSelectLevel.h"
#include "CharCreateLevel.h"
#include "Player.h"
#include "ChatWindow.h"
#include "CreateAccountLevel.h"
#include "GameMenu.h"
#include "PingDot.h"
#include "TabGroup.h"
#include "TargetWindow.h"
#include "MailWindow.h"
#include "PostProcessController.h"
#include "PartyWindow.h"
#include "GameMenu.h"
#include "HealthBar.h"
#include <chrono>
#include <ctime>
#include "Options.h"
#include "MultiLineEdit.h"
#include "NewMailWindow.h"
#include "MissionMapWindow.h"
#include "HotkeyEdit.h"
#include "SkillBarWindow.h"

#include <Urho3D/DebugNew.h>

/**
* This macro is expanded to (roughly, depending on OS) this:
*
* > int RunApplication()
* > {
* > Urho3D::SharedPtr<Urho3D::Context> context(new Urho3D::Context());
* > Urho3D::SharedPtr<className> application(new className(context));
* > return application->Run();
* > }
* >
* > int main(int argc, char** argv)
* > {
* > Urho3D::ParseArguments(argc, argv);
* > return function;
* > }
*/
#pragma warning(push)
#pragma warning( disable : 4701 4100)
URHO3D_DEFINE_APPLICATION_MAIN(ClientApp)
#pragma warning( pop )

/**
* This happens before the engine has been initialized
* so it's usually minimal code setting defaults for
* whatever instance variables you have.
* You can also do this in the Setup method.
*/
ClientApp::ClientApp(Context* context) :
    Application(context)
{
    const Vector<String>& args = GetArguments();
    decltype(args.Size()) i = 0;
    while (i < args.Size())
    {
        auto& arg = args[i];
        if (arg == "-perfpath")
        {
            ++i;
            Options::SetPrefPath(args[i].CString());
        }
        ++i;
    }

    MultiLineEdit::RegisterObject(context);

    // Register levels
    context->RegisterFactory<LoginLevel>();
    context->RegisterFactory<CreateAccountLevel>();
    context->RegisterFactory<CharSelectLevel>();
    context->RegisterFactory<CharCreateLevel>();
    context->RegisterFactory<OutpostLevel>();
    context->RegisterFactory<PvpCombatLevel>();

    shortcuts_ = new Shortcuts(context);
    context->RegisterSubsystem(shortcuts_);

    options_ = new Options(context);
    options_->Load();
    context->RegisterSubsystem(options_);

    windowManager_ = new WindowManager(context);
    context->RegisterSubsystem(windowManager_);

    client_ = new FwClient(context);
    context->RegisterSubsystem(client_);
    itemsCache_ = new ItemsCache(context);
    context->RegisterSubsystem(itemsCache_);
    levelManager_ = new LevelManager(context);
    context->RegisterSubsystem(levelManager_);

    // UI
    TabGroup::RegisterObject(context);
    HealthBar::RegisterObject(context);
    HotkeyEdit::RegisterObject(context);

    // Register factory and attributes for the Character component so it can
    // be created via CreateComponent, and loaded / saved
    Actor::RegisterObject(context);
    Player::RegisterObject(context);
    ChatWindow::RegisterObject(context);
    GameMenu::RegisterObject(context);
    PingDot::RegisterObject(context);
    TargetWindow::RegisterObject(context);
    MailWindow::RegisterObject(context);
    NewMailWindow::RegisterObject(context);
    PartyWindow::RegisterObject(context);
    MissionMapWindow::RegisterObject(context);
    OptionsWindow::RegisterObject(context);
    PostProcessController::RegisterObject(context);
    SkillBarWindow::RegisterObject(context);

#ifdef DEBUG_HUD
    SubscribeToEvent(AbEvents::E_SC_TOGGLEDEBUGHUD, URHO3D_HANDLER(ClientApp, HandleToggleDebugHUD));
    SubscribeToEvent(AbEvents::E_SC_TOGGLECONSOLE, URHO3D_HANDLER(ClientApp, HandleToggleConsole));
#endif
    SubscribeToEvent(AbEvents::E_SC_TOGGLEOPTIONS, URHO3D_HANDLER(ClientApp, HandleToggleOptions));
    SubscribeToEvent(AbEvents::E_SC_TAKESCREENSHOT, URHO3D_HANDLER(ClientApp, HandleTakeScreenshot));
    SubscribeToEvent(AbEvents::E_SC_EXITPROGRAM, URHO3D_HANDLER(ClientApp, HandleExitProgram));
}

/**
* This method is called before the engine has been initialized.
* Thusly, we can setup the engine parameters before anything else
* of engine importance happens (such as windows, search paths,
* resolution and other things that might be user configurable).
*/
void ClientApp::Setup()
{
    Options* options = GetSubsystem<Options>();
    // These parameters should be self-explanatory.
    // See http://urho3d.github.io/documentation/1.5/_main_loop.html
    // for a more complete list.
    //  Default 0 (use desktop resolution, or 1024 in windowed mode.)
    engineParameters_[EP_WINDOW_WIDTH] = options->GetWidth();
    // Default 0 (use desktop resolution, or 768 in windowed mode.)
    engineParameters_[EP_WINDOW_HEIGHT] = options->GetHeight();
    engineParameters_[EP_WINDOW_POSITION_X] = options->GetWindowPos().x_;
    engineParameters_[EP_WINDOW_POSITION_Y] = options->GetWindowPos().y_;
    engineParameters_[EP_FULL_SCREEN] = options->GetFullscreen();
    engineParameters_[EP_BORDERLESS] = options->GetBorderless();
    engineParameters_[EP_WINDOW_RESIZABLE] = options->GetResizeable();
    engineParameters_[EP_HIGH_DPI] = options->GetHighDPI();
    engineParameters_[EP_VSYNC] = options->GetVSync();
    engineParameters_[EP_TRIPLE_BUFFER] = options->GetTripleBuffer();
    engineParameters_[EP_MULTI_SAMPLE] = options->GetMultiSample();
    engineParameters_[EP_TEXTURE_FILTER_MODE] = options->GetTextureFilterMode();
    engineParameters_[EP_TEXTURE_ANISOTROPY] = options->GetTextureAnisotropyLevel();
    engineParameters_[EP_SHADOWS] = options->GetShadows();
    engineParameters_[EP_AUTOLOAD_PATHS] = "Autoload";
    engineParameters_[EP_RESOURCE_PATHS] = "AbData;GameData;CoreData;Data";
    engineParameters_[EP_LOG_NAME] = "fw.log";
#if defined(AB_CLIENT_LOGGING)
    engineParameters_[EP_LOG_QUIET] = false;
#else
    engineParameters_[EP_LOG_QUIET] = true;
#endif
    // "RenderPaths/Prepass.xml";
    // "RenderPaths/Deferred.xml";
    const String& rp = options->GetRenderPath();
    if (!rp.Empty())
        engineParameters_[EP_RENDER_PATH] = options->GetRenderPath();

    GetSubsystem<UI>()->SetUseSystemClipboard(true);
}

/**
* This method is called after the engine has been initialized.
* This is where you set up your actual content, such as scenes,
* models, controls and what not. Basically, anything that needs
* the engine initialized and ready goes in here.
*/
void ClientApp::Start()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    SetRandomSeed(Time::GetSystemTime());
    SetWindowTitleAndIcon();

    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    // Let's use the default style that comes with Urho3D.
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml"));

    Options* options = GetSubsystem<Options>();
    options->UpdateAudio();
    Renderer* renderer = GetSubsystem<Renderer>();
    // Oh what a difference!
    renderer->SetShadowQuality(options->GetShadowQuality());
    renderer->SetTextureQuality(options->GetTextureQuality());
    renderer->SetMaterialQuality(options->GetMaterialQuality());
    renderer->SetHDRRendering(true);
    renderer->SetSpecularLighting(true);

#ifdef DEBUG_HUD
    CreateHUD();
#endif

    SwitchScene("LoginLevel");
}

/**
* Good place to get rid of any system resources that requires the
* engine still initialized. You could do the rest in the destructor,
* but there's no need, this method will get called when the engine stops,
* for whatever reason (short of a segfault).
*/
void ClientApp::Stop()
{
    FwClient* cli = context_->GetSubsystem<FwClient>();
    cli->Logout();
    Options* options = GetSubsystem<Options>();
    options->Save();
    windowManager_->SaveWindows();
}

#ifdef DEBUG_HUD
void ClientApp::CreateHUD()
{
    // Use fixed width font for HUD
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xmlFile = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    // Create console
    Console* console = engine_->CreateConsole();
    console->SetDefaultStyle(xmlFile);
    console->GetBackground()->SetOpacity(0.7f);

    // Create debug HUD.
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(xmlFile);
}
#endif

void ClientApp::SetWindowTitleAndIcon()
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    Graphics* graphics = GetSubsystem<Graphics>();
    Image* icon = cache->GetResource<Image>("Textures/Icon.png");
    graphics->SetWindowIcon(icon);
    graphics->SetWindowTitle("FW");
}

void ClientApp::SwitchScene(const String& sceneName)
{
    // Switch level
    VariantMap& eventData = GetEventDataMap();
    using namespace AbEvents::SetLevel;
    eventData[P_NAME] = sceneName;
    SendEvent(AbEvents::E_SETLEVEL, eventData);
}

#ifdef DEBUG_HUD
void ClientApp::HandleToggleDebugHUD(StringHash, VariantMap&)
{
    GetSubsystem<DebugHud>()->ToggleAll();
    GetSubsystem<LevelManager>()->ToggleDebugGeometry();
}
#endif

#ifdef DEBUG_HUD
void ClientApp::HandleToggleConsole(StringHash, VariantMap&)
{
    GetSubsystem<Console>()->Toggle();
}
#endif

void ClientApp::HandleToggleOptions(StringHash, VariantMap&)
{
    WindowManager* wm = GetSubsystem<WindowManager>();
    SharedPtr<UIElement> optionsWnd = wm->GetWindow(WINDOW_OPTIONS, true);
    optionsWnd->SetVisible(!optionsWnd->IsVisible());
}

void ClientApp::HandleTakeScreenshot(StringHash, VariantMap&)
{
    Graphics* graphics = GetSubsystem<Graphics>();
    Image image(context_);
    graphics->TakeScreenShot(image);
    String path = AddTrailingSlash(Options::GetPrefPath()) + "screens/";

    if (!Options::CreateDir(path))
    {
        URHO3D_LOGERRORF("Failed to create directory %s", path);
        return;
    }
    std::chrono::time_point<std::chrono::system_clock> time_point;
    time_point = std::chrono::system_clock::now();
    std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
    tm p;
    localtime_s(&p, &ttp);
    char chr[50];
    strftime(chr, 50, "%Y-%m-%d-%H-%M-%S", (const tm*)&p);

    String file = path + "fw" + String(chr) + ".png";
    image.SavePNG(file);

    using namespace AbEvents::ScreenshotTaken;
    VariantMap& e = GetEventDataMap();
    e[P_FILENAME] = file;
    SendEvent(AbEvents::E_SCREENSHOTTAKEN, e);
}

void ClientApp::HandleExitProgram(StringHash, VariantMap&)
{
    Engine* engine = context_->GetSubsystem<Engine>();
    engine->Exit();
}
