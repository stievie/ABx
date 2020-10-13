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


#include "EquipmentWindow.h"
#include "Shortcuts.h"
#include "ShortcutEvents.h"
#include "LevelManager.h"
#include "Player.h"
#include "ItemsCache.h"
#include "PostProcessController.h"
#include "BaseLevel.h"

EquipmentWindow::EquipmentWindow(Context* context) :
    Window(context)
{
    SetName("EquipmentWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/EquipmentWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("CaptionText", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLEEQUIPWINDOW, "Equipment", true));

    SetSize(330, 420);
    SetPosition(10, 30);
    SetVisible(true);

    SetStyleAuto();

    modelViewer_ = GetChildStaticCast<View3D>("Model", true);
    modelScene_ = new Scene(context);
    auto* sceneFile = cache->GetResource<XMLFile>("Scenes/EquipmentScene.xml");
    if (sceneFile)
    {
        modelScene_->LoadXML(sceneFile->GetRoot());
        Camera* camera = modelScene_->GetComponent<Camera>(true);
        modelViewer_->SetView(modelScene_, camera, false);
        characterNode_ = modelScene_->CreateChild(0, LOCAL);
    }
    else
        URHO3D_LOGERROR("Scene not found 'Scenes/EquipmentScene.xml'");

    SubscribeEvents();
}

EquipmentWindow::~EquipmentWindow()
{
    UnsubscribeFromAllEvents();
}

void EquipmentWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleCloseClicked));
    Button* loadButton = GetChildStaticCast<Button>("LoadButton", true);
    SubscribeToEvent(loadButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleLoadClicked));
    Button* saveButton = GetChildStaticCast<Button>("SaveButton", true);
    SubscribeToEvent(saveButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleSaveClicked));
    SubscribeToEvent(E_MOUSEMOVE, URHO3D_HANDLER(EquipmentWindow, HandleSceneViewerMouseMove));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(EquipmentWindow, HandleSceneViewerMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(EquipmentWindow, HandleSceneViewerMouseUp));

    Button* createButton = GetChildStaticCast<Button>("CreateButton", true);
    createButton->SetVisible(false);
}

void EquipmentWindow::HandleSceneViewerMouseMove(StringHash, VariantMap& eventData)
{
    using namespace MouseMove;
    if (!mouseDown_)
        return;
    const int deltaX = eventData[P_DX].GetInt();
    const Quaternion& rot = characterNode_->GetRotation();
    const float r = rot.EulerAngles().y_ - (float)deltaX;
    const Quaternion newRot = Quaternion(r, Vector3::UP);
    characterNode_->SetRotation(newRot);
}

void EquipmentWindow::HandleSceneViewerMouseDown(StringHash, VariantMap& eventData)
{
    using namespace MouseButtonDown;
    if (eventData[P_BUTTON].GetUInt() == MOUSEB_LEFT)
    {
        auto* input = GetSubsystem<Input>();
        if (modelViewer_->IsInside(input->GetMousePosition(), true))
            mouseDown_ = true;
    }
}

void EquipmentWindow::HandleSceneViewerMouseUp(StringHash, VariantMap& eventData)
{
    using namespace MouseButtonUp;
    if (eventData[P_BUTTON].GetUInt() == MOUSEB_LEFT && mouseDown_)
        mouseDown_ = false;
}

void EquipmentWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void EquipmentWindow::HandleLoadClicked(StringHash, VariantMap&)
{
}

void EquipmentWindow::HandleSaveClicked(StringHash, VariantMap&)
{
}

void EquipmentWindow::HandleCreateClicked(StringHash, VariantMap&)
{
}

bool EquipmentWindow::LoadObject(uint32_t itemIndex, Node* node)
{
    ItemsCache* items = GetSubsystem<ItemsCache>();
    SharedPtr<Item> item = items->Get(itemIndex);
    if (!item)
    {
        URHO3D_LOGERRORF("Model Item not found: %d", itemIndex);
        return false;
    }
    XMLFile* object = item->GetObjectResource<XMLFile>();
    if (!object)
    {
        URHO3D_LOGERRORF("Prefab file not found for %s: %s", item->name_.CString(), item->objectFile_.CString());
        return false;
    }

    XMLElement root = object->GetRoot();
    unsigned nodeId = root.GetUInt("id");
    SceneResolver resolver;
    Node* adjNode = node->CreateChild(0, LOCAL);
    resolver.AddNode(nodeId, adjNode);
    adjNode->SetRotation(Quaternion(90, Vector3(0, 1, 0)));
    if (adjNode->LoadXML(root, resolver, true, true))
    {
        resolver.Resolve();
        adjNode->ApplyAttributes();
        animController_ = adjNode->CreateComponent<AnimationController>();
        String idleAnimation = Actor::GetAnimation(item->modelClass_, ANIM_IDLE);
        if (!idleAnimation.Empty())
            animController_->PlayExclusive(idleAnimation, 0, true, 0.0);
    }
    else
    {
        URHO3D_LOGERRORF("Error instantiating prefab %s", item->objectFile_.CString());
        adjNode->Remove();
        return false;
    }
    return true;
}

void EquipmentWindow::UpdateEquipment(Player* player)
{
    if (!player)
        return;

    Button* createButton = GetChildStaticCast<Button>("CreateButton", true);
    if (player->pvpCharacter_)
    {
        createButton->SetVisible(true);
        SubscribeToEvent(createButton, E_RELEASED, URHO3D_HANDLER(EquipmentWindow, HandleCreateClicked));
    }
    else
    {
        createButton->SetVisible(false);
    }

    if (!modelLoaded_)
    {
        if  (LoadObject(player->itemIndex_, characterNode_))
            modelLoaded_ = true;
    }
}

void EquipmentWindow::Initialize(PostProcessController& pp)
{
    if (!initialized_)
    {
        pp.AddViewport(modelViewer_->GetViewport());
        initialized_ = true;
    }
}
