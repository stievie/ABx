//
// Copyright (c) 2008-2016 the Urho3D project.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "stdafx.h"
#include <string>
#include <sstream>

#include "abclient.h"

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
    Application(context),
    framecount_(0),
    time_(0)
{
}

/**
* This method is called before the engine has been initialized.
* Thusly, we can setup the engine parameters before anything else
* of engine importance happens (such as windows, search paths,
* resolution and other things that might be user configurable).
*/
void ClientApp::Setup()
{
    // These parameters should be self-explanatory.
    // See http://urho3d.github.io/documentation/1.5/_main_loop.html
    // for a more complete list.
    engineParameters_["FullScreen"] = false;
    engineParameters_["WindowWidth"] = 1280;
    engineParameters_["WindowHeight"] = 720;
    engineParameters_["WindowResizable"] = true;
}

/**
* This method is called after the engine has been initialized.
* This is where you set up your actual content, such as scenes,
* models, controls and what not. Basically, anything that needs
* the engine initialized and ready goes in here.
*/
void ClientApp::Start()
{
    // We will be needing to load resources.
    // All the resources used in this example comes with Urho3D.
    // If the engine can't find them, check the ResourcePrefixPath (see http://urho3d.github.io/documentation/1.5/_main_loop.html).
    ResourceCache* cache = GetSubsystem<ResourceCache>();

    // Let's use the default style that comes with Urho3D.
    GetSubsystem<UI>()->GetRoot()->SetDefaultStyle(cache->GetResource<XMLFile>("UI/DefaultStyle.xml"));
    // Let's create some text to display.
    text_ = new Text(context_);
    // Text will be updated later in the E_UPDATE handler. Keep readin'.
    text_->SetText("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\nWait a bit to see FPS.");
    // If the engine cannot find the font, it comes with Urho3D.
    // Set the environment variables URHO3D_HOME, URHO3D_PREFIX_PATH or
    // change the engine parameter "ResourcePrefixPath" in the Setup method.
    text_->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 20);
    text_->SetColor(Color(.3, 0, .3));
    text_->SetHorizontalAlignment(HA_CENTER);
    text_->SetVerticalAlignment(VA_TOP);
    GetSubsystem<UI>()->GetRoot()->AddChild(text_);
    // Add a button, just as an interactive UI sample.
    Button* button = new Button(context_);
    // Note, must be part of the UI system before SetSize calls!
    GetSubsystem<UI>()->GetRoot()->AddChild(button);
    button->SetName("Button Quit");
    button->SetStyle("Button");
    button->SetSize(32, 32);
    button->SetPosition(16, 116);
    // Subscribe to button release (following a 'press') events
    SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(ClientApp, HandleClosePressed));

    // Let's setup a scene to render.
    scene_ = new Scene(context_);
    // Let the scene have an Octree component!
    scene_->CreateComponent<Octree>();
    // Let's add an additional scene component for fun.
    scene_->CreateComponent<DebugRenderer>();

    // Let's put some sky in there.
    // Again, if the engine can't find these resources you need to check
    // the "ResourcePrefixPath". These files come with Urho3D.
    Node* skyNode = scene_->CreateChild("Sky");
    skyNode->SetScale(500.0f); // The scale actually does not matter
    Skybox* skybox = skyNode->CreateComponent<Skybox>();
    skybox->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    skybox->SetMaterial(cache->GetResource<Material>("Materials/Skybox.xml"));

    // Let's put a box in there.
    boxNode_ = scene_->CreateChild("Box");
    boxNode_->SetPosition(Vector3(0, 2, 15));
    boxNode_->SetScale(Vector3(3, 3, 3));
    StaticModel* boxObject = boxNode_->CreateComponent<StaticModel>();
    boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
    boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
    boxObject->SetCastShadows(true);

    // Create 400 boxes in a grid.
    for(int x = -30; x < 30; x += 3)
        for(int z = 0; z < 60; z += 3)
        {
            Node* boxNode_ = scene_->CreateChild("Box");
            boxNode_->SetPosition(Vector3(x, -3, z));
            boxNode_->SetScale(Vector3(2, 2, 2));
            StaticModel* boxObject = boxNode_->CreateComponent<StaticModel>();
            boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
            boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
            boxObject->SetCastShadows(true);
        }

    // We need a camera from which the viewport can render.
    cameraNode_ = scene_->CreateChild("Camera");
    Camera* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(2000);

    // Create a red directional light (sun)
    {
        Node* lightNode = scene_->CreateChild();
        lightNode->SetDirection(Vector3::FORWARD);
        lightNode->Yaw(50);     // horizontal
        lightNode->Pitch(10);   // vertical
        Light* light=lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetBrightness(1.6);
        light->SetColor(Color(1.0, .6, 0.3, 1));
        light->SetCastShadows(true);
    }
    // Create a blue point light
    {
        Node* lightNode = scene_->CreateChild("Light");
        lightNode->SetPosition(Vector3(-10, 2, 5));
        Light* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(25);
        light->SetBrightness(1.7);
        light->SetColor(Color(0.5, .5, 1.0, 1));
        light->SetCastShadows(true);
    }
    // add a green spot light to the camera node
    {
        Node* node_light = cameraNode_->CreateChild();
        Light* light = node_light->CreateComponent<Light>();
        node_light->Pitch(15);  // point slightly downwards
        light->SetLightType(LIGHT_SPOT);
        light->SetRange(20);
        light->SetColor(Color(.6, 1, .6, 1.0));
        light->SetBrightness(2.8);
        light->SetFov(25);
    }

    // Now we setup the viewport. Of course, you can have more than one!
    Renderer* renderer = GetSubsystem<Renderer>();
    SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
    renderer->SetViewport(0, viewport);

    // We subscribe to the events we'd like to handle.
    // In this example we will be showing what most of them do,
    // but in reality you would only subscribe to the events
    // you really need to handle.
    // These are sort of subscribed in the order in which the engine
    // would send the events. Read each handler method's comment for
    // details.
    SubscribeToEvent(E_BEGINFRAME, URHO3D_HANDLER(ClientApp, HandleBeginFrame));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ClientApp, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(ClientApp, HandleUpdate));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(ClientApp, HandlePostUpdate));
    SubscribeToEvent(E_RENDERUPDATE, URHO3D_HANDLER(ClientApp, HandleRenderUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(ClientApp, HandlePostRenderUpdate));
    SubscribeToEvent(E_ENDFRAME, URHO3D_HANDLER(ClientApp, HandleEndFrame));
}

/**
* Good place to get rid of any system resources that requires the
* engine still initialized. You could do the rest in the destructor,
* but there's no need, this method will get called when the engine stops,
* for whatever reason (short of a segfault).
*/
void ClientApp::Stop()
{
}

/**
* Every frame's life must begin somewhere. Here it is.
*/
void ClientApp::HandleBeginFrame(StringHash eventType, VariantMap& eventData)
{
    // We really don't have anything useful to do here for this example.
    // Probably shouldn't be subscribing to events we don't care about.
}

/**
* Input from keyboard is handled here. I'm assuming that Input, if
* available, will be handled before E_UPDATE.
*/
void ClientApp::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;
    int key = eventData[P_KEY].GetInt();
    if(key == KEY_ESCAPE)
        engine_->Exit();

    if(key == KEY_TAB)    // toggle mouse cursor when pressing tab
    {
        GetSubsystem<Input>()->SetMouseVisible(!GetSubsystem<Input>()->IsMouseVisible());
        GetSubsystem<Input>()->SetMouseGrabbed(!GetSubsystem<Input>()->IsMouseGrabbed());
    }
}

/**
* You can get these events from when ever the user interacts with the UI.
*/
void ClientApp::HandleClosePressed(StringHash eventType, VariantMap& eventData)
{
    engine_->Exit();
}

/**
* Your non-rendering logic should be handled here.
* This could be moving objects, checking collisions and reaction, etc.
*/
void ClientApp::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    float timeStep = eventData[Update::P_TIMESTEP].GetFloat();
    framecount_++;
    time_ += timeStep;
    // Movement speed as world units per second
    float MOVE_SPEED = 10.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    if(time_ >= 1)
    {
        std::string str;
        str.append("Keys: tab = toggle mouse, AWSD = move camera, Shift = fast mode, Esc = quit.\n");
        {
            std::ostringstream ss;
            ss << framecount_;
            std::string s(ss.str());
            str.append(s.substr(0, 6));
        }
        str.append(" frames in ");
        {
            std::ostringstream ss;
            ss << time_;
            std::string s(ss.str());
            str.append(s.substr(0, 6));
        }
        str.append(" seconds = ");
        {
            std::ostringstream ss;
            ss << (float)framecount_ / time_;
            std::string s(ss.str());
            str.append(s.substr(0, 6));
        }
        str.append(" fps");
        String s(str.c_str(), str.size());
        text_->SetText(s);
        URHO3D_LOGINFO(s);     // this show how to put stuff into the log
        framecount_ = 0;
        time_ = 0;
    }

    // Rotate the box thingy.
    // A much nicer way of doing this would be with a LogicComponent.
    // With LogicComponents it is easy to control things like movement
    // and animation from some IDE, console or just in game.
    // Alas, it is out of the scope for our simple example.
    boxNode_->Rotate(Quaternion(8 * timeStep, 16 * timeStep, 0));

    Input* input=GetSubsystem<Input>();
    if(input->GetQualifierDown(1))  // 1 is shift, 2 is ctrl, 4 is alt
        MOVE_SPEED *= 10;
    if(input->GetKeyDown('W'))
        cameraNode_->Translate(Vector3(0, 0, 1) * MOVE_SPEED * timeStep);
    if(input->GetKeyDown('S'))
        cameraNode_->Translate(Vector3(0, 0, -1) * MOVE_SPEED * timeStep);
    if(input->GetKeyDown('A'))
        cameraNode_->Translate(Vector3(-1, 0, 0) * MOVE_SPEED * timeStep);
    if(input->GetKeyDown('D'))
        cameraNode_->Translate(Vector3( 1, 0, 0) * MOVE_SPEED * timeStep);

    if(!GetSubsystem<Input>()->IsMouseVisible())
    {
        // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
        IntVector2 mouseMove = input->GetMouseMove();
        static float yaw_ = 0;
        static float pitch_ = 0;
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);
        // Reset rotation and set yaw and pitch again
        cameraNode_->SetDirection(Vector3::FORWARD);
        cameraNode_->Yaw(yaw_);
        cameraNode_->Pitch(pitch_);
    }
}

/**
* Anything in the non-rendering logic that requires a second pass,
* it might be well suited to be handled here.
*/
void ClientApp::HandlePostUpdate(StringHash eventType, VariantMap& eventData)
{
    // We really don't have anything useful to do here for this example.
    // Probably shouldn't be subscribing to events we don't care about.
}

/**
* If you have any details you want to change before the viewport is
* rendered, try putting it here.
* See http://urho3d.github.io/documentation/1.32/_rendering.html
* for details on how the rendering pipeline is setup.
*/
void ClientApp::HandleRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    // We really don't have anything useful to do here for this example.
    // Probably shouldn't be subscribing to events we don't care about.
}

/**
* After everything is rendered, there might still be things you wish
* to add to the rendering. At this point you cannot modify the scene,
* only post rendering is allowed. Good for adding things like debug
* artifacts on screen or brush up lighting, etc.
*/
void ClientApp::HandlePostRenderUpdate(StringHash eventType, VariantMap & eventData)
{
    // We could draw some debuggy looking thing for the octree.
    // scene_->GetComponent<Octree>()->DrawDebugGeometry(true);
}

/**
* All good things must come to an end.
*/
void ClientApp::HandleEndFrame(StringHash eventType, VariantMap& eventData)
{
    // We really don't have anything useful to do here for this example.
    // Probably shouldn't be subscribing to events we don't care about.
}
