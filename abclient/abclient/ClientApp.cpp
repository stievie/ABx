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
#include "MultiLineEdit.h"
#include "MailWindow.h"
#include "PostProcessController.h"

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
URHO3D_DEFINE_APPLICATION_MAIN(ClientApp)

/**
* This happens before the engine has been initialized
* so it's usually minimal code setting defaults for
* whatever instance variables you have.
* You can also do this in the Setup method.
*/
ClientApp::ClientApp(Context* context) :
    Application(context)
{
    // Register levels
    context->RegisterFactory<LoginLevel>();
    context->RegisterFactory<CreateAccountLevel>();
    context->RegisterFactory<CharSelectLevel>();
    context->RegisterFactory<CharCreateLevel>();
    context->RegisterFactory<OutpostLevel>();
    context->RegisterFactory<PvpCombatLevel>();

    options_ = new Options(context);
    options_->Load();
    context->RegisterSubsystem(options_);

    client_ = new FwClient(context);
    context->RegisterSubsystem(client_);
    levelManager_ = new LevelManager(context);
    context->RegisterSubsystem(levelManager_);

    // UI
    TabGroup::RegisterObject(context);
    MultiLineEdit::RegisterObject(context);

    // Register factory and attributes for the Character component so it can
    // be created via CreateComponent, and loaded / saved
    Actor::RegisterObject(context);
    Player::RegisterObject(context);
    ChatWindow::RegisterObject(context);
    GameMenu::RegisterObject(context);
    PingDot::RegisterObject(context);
    TargetWindow::RegisterObject(context);
    MailWindow::RegisterObject(context);
    PostProcessController::RegisterObject(context);

    // Subscribe key down event
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ClientApp, HandleKeyDown));
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
    engineParameters_[EP_FULL_SCREEN] = options->GetFullscreen();
    engineParameters_[EP_BORDERLESS] = options->GetBorderless();
    engineParameters_[EP_WINDOW_RESIZABLE] = options->GetResizeable();
    engineParameters_[EP_HIGH_DPI] = options->GetHighDPI();
    engineParameters_[EP_VSYNC] = options->GetVSync();
    engineParameters_[EP_TRIPLE_BUFFER] = options->GetTripleBuffer();
    engineParameters_[EP_MULTI_SAMPLE] = options->GetMultiSample();
    engineParameters_[EP_AUTOLOAD_PATHS] = "Autoload";
    engineParameters_[EP_RESOURCE_PATHS] = "CoreData;Data;AbData";
    engineParameters_[EP_LOG_NAME] = "abclient.log";
    engineParameters_[EP_LOG_QUIET] = false;
    // "RenderPaths/Prepass.xml";
    // "RenderPaths/Deferred.xml";
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
    SetRandomSeed(Time::GetSystemTime());
    SetWindowTitleAndIcon();

    GetSubsystem<Input>()->SetMouseVisible(true);
    GetSubsystem<Input>()->SetMouseGrabbed(false);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    // Let's use the default style that comes with Urho3D.
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml"));

    GetSubsystem<Options>()->UpdateAudio();

    // We subscribe to the events we'd like to handle.
    // In this example we will be showing what most of them do,
    // but in reality you would only subscribe to the events
    // you really need to handle.
    // These are sort of subscribed in the order in which the engine
    // would send the events. Read each handler method's comment for
    // details.
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ClientApp, HandleUpdate));

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
}

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
    eventData[AbEvents::E_SET_LEVEL] = sceneName;
    SendEvent(AbEvents::E_SET_LEVEL, eventData);
}

void ClientApp::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();

//#ifdef _DEBUG
    // Toggle debug HUD with F2
    if (key == KEY_F2)
    {
        GetSubsystem<LevelManager>()->ToggleDebugGeometry();
    }
//#endif
}

/**
* Your non-rendering logic should be handled here.
* This could be moving objects, checking collisions and reaction, etc.
*/
void ClientApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    Input* input = GetSubsystem<Input>();

    if (input->GetKeyPress(KEY_F11))
    {
        Options* options = GetSubsystem<Options>();
        options->SetFullscreen(!options->GetFullscreen());
    }
}
