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



#include "ClientApp.h"
#include "ActorResourceBar.h"
#include "CameraTransform.h"
#include "CharCreateLevel.h"
#include "CharSelectLevel.h"
#include "ChatFilter.h"
#include "ChatWindow.h"
#include "ClientPrediction.h"
#include "CraftsmanWindow.h"
#include "CreateAccountLevel.h"
#include "CreditsWindow.h"
#include "DamageWindow.h"
#include "EffectsWindow.h"
#include "FadeWindow.h"
#include "FormatText.h"
#include "FriendListWindow.h"
#include "GameMenu.h"
#include "GameMessagesWindow.h"
#include "HealthBar.h"
#include "HotkeyEdit.h"
#include "InventoryWindow.h"
#include "ItemStatsUIElement.h"
#include "ItemUIElement.h"
#include "LoginLevel.h"
#include "MailWindow.h"
#include "MerchantWindow.h"
#include "MissionMapWindow.h"
#include "MultiLineEdit.h"
#include "NewMailWindow.h"
#include "Options.h"
#include "OutpostLevel.h"
#include "PartyItem.h"
#include "PartyWindow.h"
#include "PingDot.h"
#include "Player.h"
#include "PostProcessController.h"
#include "PriceUIElement.h"
#include "PvpCombatLevel.h"
#include "SkillBarWindow.h"
#include "SkillCostElement.h"
#include "Spinner.h"
#include "TabGroup.h"
#include "TargetWindow.h"
#include "UpdateProgressWindow.h"
#include <asio/detail/config.hpp>
#include <asio/version.hpp>
#include <chrono>
#include <ctime>
#include <stdlib.h>
#include <time.h>
#if defined(__linux__)
#include <linux/version.h>
#endif
#include <sa/Compiler.h>
#include <sa/Process.h>
#include "Conversions.h"

//#include <Urho3D/DebugNew.h>

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
PRAGMA_WARNING_PUSH
PRAGMA_WARNING_DISABLE_MSVC(4701 4100)
URHO3D_DEFINE_APPLICATION_MAIN(ClientApp)
PRAGMA_WARNING_POP

bool gNoClientPrediction = false;

/**
* This happens before the engine has been initialized
* so it's usually minimal code setting defaults for
* whatever instance variables you have.
* You can also do this in the Setup method.
*/
ClientApp::ClientApp(Context* context) :
    Application(context)
{
    options_ = MakeShared<Options>(context);

    exeName_ = ToUrhoString(sa::Process::GetSelf());
    appPath_ = ToUrhoString(sa::Process::GetSelfPath());

    std::vector<std::string> procArgs;
    const Vector<String>& args = GetArguments();
    decltype(args.Size()) i = 0;
    while (i < args.Size())
    {
        auto& arg = args[i];
        procArgs.push_back(ToStdString(arg));
        if (arg == "-perfpath")
        {
            ++i;
            if (i < args.Size())
                Options::SetPrefPath(args[i].CString());
        }
        else if (arg == "-username")
        {
            ++i;
            if (i < args.Size())
                options_->username_ = args[i];
        }
        else if (arg == "-password")
        {
            ++i;
            if (i < args.Size())
                options_->password_ = args[i];
        }
        else if (arg == "-no-cp")
        {
            gNoClientPrediction = true;
        }
        ++i;
    }
    process_ = std::make_unique<sa::Process>(procArgs);

    MultiLineEdit::RegisterObject(context);
    FormatText::RegisterObject(context);

    context->RegisterFactory<CameraTransform>();
    context->RegisterFactory<FadeWindow>();
    context->RegisterFactory<UpdateProgressWindow>();
    // Register levels
    context->RegisterFactory<LoginLevel>();
    context->RegisterFactory<CreateAccountLevel>();
    context->RegisterFactory<CharSelectLevel>();
    context->RegisterFactory<CharCreateLevel>();
    context->RegisterFactory<OutpostLevel>();
    context->RegisterFactory<PvpCombatLevel>();

    shortcuts_ = MakeShared<Shortcuts>(context);
    context->RegisterSubsystem(shortcuts_);

    options_->Load();
    context->RegisterSubsystem(options_);

    windowManager_ = MakeShared<WindowManager>(context);
    context->RegisterSubsystem(windowManager_);

    auto* chatFilter = new ChatFilter(context);
    chatFilter->Load();
    context->RegisterSubsystem(chatFilter);

    client_ = MakeShared<FwClient>(context);
    context->RegisterSubsystem(client_);
    itemsCache_ = MakeShared<ItemsCache>(context);
    context->RegisterSubsystem(itemsCache_);
    skillsManager_ = MakeShared<SkillManager>(context);
    context->RegisterSubsystem(skillsManager_);
    levelManager_ = MakeShared<LevelManager>(context);
    context->RegisterSubsystem(levelManager_);
    audioManager_ = MakeShared<AudioManager>(context);
    context->RegisterSubsystem(audioManager_);
    mumble_ = MakeShared<Mumble>(context);
    context->RegisterSubsystem(mumble_);

    // UI
    TabGroup::RegisterObject(context);
    HealthBar::RegisterObject(context);
    ValueBar::RegisterObject(context);
    HotkeyEdit::RegisterObject(context);
    Spinner::RegisterObject(context);
    PartyItem::RegisterObject(context);
    SkillCostElement::RegisterObject(context);
    ItemUIElement::RegisterObject(context);
    PriceUIElement::RegisterObject(context);
    ItemStatsUIElement::RegisterObject(context);

    // Register factory and attributes for the Character component so it can
    // be created via CreateComponent, and loaded / saved
    Actor::RegisterObject(context);
    Player::RegisterObject(context);
    ClientPrediction::RegisterObject(context);
    ChatWindow::RegisterObject(context);
    CraftsmanWindow::RegisterObject(context);
    GameMenu::RegisterObject(context);
    PingDot::RegisterObject(context);
    TargetWindow::RegisterObject(context);
    MailWindow::RegisterObject(context);
    MerchantWindow::RegisterObject(context);
    NewMailWindow::RegisterObject(context);
    PartyWindow::RegisterObject(context);
    MissionMapWindow::RegisterObject(context);
    OptionsWindow::RegisterObject(context);
    PostProcessController::RegisterObject(context);
    SkillBarWindow::RegisterObject(context);
    ActorResourceBar::RegisterObject(context);
    FriendListWindow::RegisterObject(context);
    GameMessagesWindow::RegisterObject(context);
    EffectsWindow::RegisterObject(context);
    CreditsWindow::RegisterObject(context);
    InventoryWindow::RegisterObject(context);
    DamageWindow::RegisterObject(context);

#ifdef DEBUG_HUD
    SubscribeToEvent(Events::E_SC_TOGGLEDEBUGHUD, URHO3D_HANDLER(ClientApp, HandleToggleDebugHUD));
    SubscribeToEvent(Events::E_SC_TOGGLECONSOLE, URHO3D_HANDLER(ClientApp, HandleToggleConsole));
#endif
    SubscribeToEvent(Events::E_SC_TOGGLEOPTIONS, URHO3D_HANDLER(ClientApp, HandleToggleOptions));
    SubscribeToEvent(Events::E_SC_TAKESCREENSHOT, URHO3D_HANDLER(ClientApp, HandleTakeScreenshot));
    SubscribeToEvent(Events::E_SC_EXITPROGRAM, URHO3D_HANDLER(ClientApp, HandleExitProgram));
    SubscribeToEvent(Events::E_SC_TOGGLEMUTEAUDIO, URHO3D_HANDLER(ClientApp, HandleToggleMuteAudio));
    SubscribeToEvent(Events::E_START_PROGRAM, URHO3D_HANDLER(ClientApp, HandleStartProgram));
    SubscribeToEvent(Events::E_RESTART, URHO3D_HANDLER(ClientApp, HandleRestart));
}

ClientApp::~ClientApp()
{
    VariantMap& eData = GetEventDataMap();
    SendEvent(Events::E_CANCELUPDATE, eData);
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
    // The directory for data files downloaded from the server.
    String gameDataDir = AddTrailingSlash(options->GetPrefPath()) + "GameData";
    if (!Options::CreateDir(gameDataDir))
    {
        ErrorExit("Unable to create directory " + gameDataDir);
    }

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
    engineParameters_[EP_RESOURCE_PREFIX_PATHS] = "./;" + options->GetPrefPath();
    engineParameters_[EP_RESOURCE_PATHS] = AB_CLIENT_RESOURSES;
    engineParameters_[EP_LOG_NAME] = "fw.log";
#if defined(AB_CLIENT_LOGGING)
    engineParameters_[EP_LOG_QUIET] = false;
#else
    engineParameters_[EP_LOG_QUIET] = true;
#endif

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

    auto* input = GetSubsystem<Input>();
    input->SetMouseMode(MM_ABSOLUTE);
    input->SetMouseVisible(true);
    input->SetMouseGrabbed(false);

    // Let's use the default style that comes with Urho3D.
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml"));

    Options* options = GetSubsystem<Options>();
    options->UpdateAudio();
    if (options->IsMaximized())
    {
        GetSubsystem<Graphics>()->Maximize();
    }
    if (options->enableMumble_)
        GetSubsystem<Mumble>()->Initialize();

    Renderer* renderer = GetSubsystem<Renderer>();
    // Oh what a difference!
    renderer->SetShadowQuality(options->GetShadowQuality());
    renderer->SetTextureQuality(options->GetTextureQuality());
    renderer->SetMaterialQuality(options->GetMaterialQuality());
    renderer->SetHDRRendering(options->GetHDRRendering());
    renderer->SetSpecularLighting(options->GetSpecularLightning());
    renderer->SetShadowSoftness(options->GetShadowSoftness());
    // https://discourse.urho3d.io/t/shadow-on-slopes/4629/5
    renderer->SetShadowMapSize(options->GetShadowMapSize());
    Engine* engine = GetSubsystem<Engine>();
    engine->SetMaxFps(options->GetMaxFps());

#ifdef DEBUG_HUD
    CreateHUD();
#endif

#ifdef _DEBUG
    std::string output;
#if defined(ASIO_HAS_IOCP)
    output += " iocp";
#endif
#if defined(ASIO_HAS_EPOLL)
    output += " epoll";
#endif
#if defined(ASIO_HAS_EVENTFD)
    output += " eventfd";
#endif
#if defined(ASIO_HAS_TIMERFD)
    output += " timerfd";
#endif
#if defined(ASIO_HAS_KQUEUE)
    output += " kqueue";
#endif
#if defined(ASIO_HAS_DEV_POLL)
    output += " /dev/poll" ;
#endif

#if defined(__linux__)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,45)
    URHO3D_LOGINFOF("LINUX_VERSION_CODE >= 2.5.45, %d", LINUX_VERSION_CODE);
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)
    URHO3D_LOGINFOF("LINUX_VERSION_CODE >= 2.6.22, %d", LINUX_VERSION_CODE);
#endif
#endif  // defined(__linux__)

    URHO3D_LOGINFOF("Asio %d has %s", ASIO_VERSION, output.c_str());
#endif  // _DEBUG

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
    FwClient* cli = GetSubsystem<FwClient>();
    cli->Stop();
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
    console->SetFocusOnShow(false);
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
    using namespace Events::SetLevel;
    eventData[P_NAME] = sceneName;
    SendEvent(Events::E_SETLEVEL, eventData);
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
        URHO3D_LOGERRORF("Failed to create directory %s", path.CString());
        return;
    }
    std::chrono::time_point<std::chrono::system_clock> time_point;
    time_point = std::chrono::system_clock::now();
    std::time_t ttp = std::chrono::system_clock::to_time_t(time_point);
    char chr[50];
#ifdef _MSC_VER
    tm p;
    localtime_s(&p, &ttp);
    strftime(chr, 50, "%Y-%m-%d-%H-%M-%S", (const tm*)&p);
#else
    tm* p;
    p = localtime(&ttp);
    strftime(chr, 50, "%Y-%m-%d-%H-%M-%S", p);
#endif

    String file = path + "fw" + String(chr) + ".png";
    image.SavePNG(file);

    using namespace Events::ScreenshotTaken;
    VariantMap& e = GetEventDataMap();
    e[P_FILENAME] = file;
    SendEvent(Events::E_SCREENSHOTTAKEN, e);
}

void ClientApp::HandleExitProgram(StringHash, VariantMap&)
{
    Engine* engine = GetSubsystem<Engine>();
    engine->Exit();
}

void ClientApp::HandleToggleMuteAudio(StringHash, VariantMap&)
{
    Options* opt = GetSubsystem<Options>();
    opt->MuteAudio();
}

void ClientApp::HandleStartProgram(StringHash, VariantMap& eventData)
{
    using namespace Events::StartProgram;
    const String& cmd = eventData[P_COMMAND].GetString();
    sa::Process::Run(ToStdString(cmd));
}

void ClientApp::HandleRestart(StringHash, VariantMap&)
{
    if (!process_)
        return;
    process_->Restart();
}
