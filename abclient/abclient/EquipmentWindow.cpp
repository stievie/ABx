#include "stdafx.h"
#include "EquipmentWindow.h"
#include "Shortcuts.h"
#include "ShortcutEvents.h"
#include "LevelManager.h"
#include "Player.h"
#include "ItemsCache.h"

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

    modelScene_ = new Scene(context);
    auto* sceneFile = cache->GetResource<XMLFile>("Scenes/EquipmentScene.xml");
    if (sceneFile)
    {
        modelScene_->LoadXML(sceneFile->GetRoot());
        Camera* camera = modelScene_->GetComponent<Camera>(true);
        View3D* modelViewer = GetChildStaticCast<View3D>("Model", true);
        modelViewer->SetView(modelScene_, camera, false);
    }
    else
        URHO3D_LOGERROR("Scene %s not found 'Scenes/EquipmentScene.xml'");

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
}

void EquipmentWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
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

    Node* node = modelScene_->GetNode(16777649);

    LoadObject(player->itemIndex_, node);
}
