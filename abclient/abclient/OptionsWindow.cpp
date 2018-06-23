#include "stdafx.h"
#include "OptionsWindow.h"

void OptionsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<OptionsWindow>();
}

OptionsWindow::OptionsWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* partyFile = cache->GetResource<XMLFile>("UI/OptionsWindow.xml");
    LoadXML(partyFile->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetName("PartyWindow");
    SetPivot(0, 0);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));

    UIElement* container = dynamic_cast<UIElement*>(GetChild("Container", true));

    tabgroup_ = container->CreateChild<TabGroup>();
    tabgroup_->SetSize(container->GetSize());
    tabgroup_->SetPosition(0, 0);

    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_TOP);
    tabgroup_->SetColor(Color(0, 0, 0, 0));
    tabgroup_->SetStyleAuto();

    {
        TabElement* elem = CreateTab(tabgroup_, "General");
        CreatePageGeneral(elem);
    }
    CreateTab(tabgroup_, "Graphics");
    CreateTab(tabgroup_, "Audio");
    CreateTab(tabgroup_, "Input");

    tabgroup_->SetEnabled(true);
    SubscribeToEvent(E_TABSELECTED, URHO3D_HANDLER(OptionsWindow, HandleTabSelected));

    // TODO: Load size/position from settings
    SetSize(400, 433);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() / 2 - (GetWidth() / 2), graphics->GetHeight() / 2 - (GetHeight() / 2));
    SetVisible(true);

    SetStyleAuto();
    UpdateLayout();

    SubscribeEvents();
}

OptionsWindow::~OptionsWindow()
{
    UnsubscribeFromAllEvents();
}

void OptionsWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    SetVisible(false);
}

void OptionsWindow::HandleTabSelected(StringHash eventType, VariantMap& eventData)
{
}

void OptionsWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(OptionsWindow, HandleCloseClicked));
}

TabElement* OptionsWindow::CreateTab(TabGroup* tabs, const String& page)
{
    static const IntVector2 tabSize(60, 20);
    static const IntVector2 tabBodySize(500, 380);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    TabElement *tabElement = tabs->CreateTab(tabSize, tabBodySize);
    tabElement->tabText_->SetFont(cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf"), 10);
    tabElement->tabText_->SetText(page);
    tabElement->tabBody_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabElement->tabBody_->SetEnableAnchor(true);
    return tabElement;
}

void OptionsWindow::CreatePageGeneral(TabElement* tabElement)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/OptionPageGeneral.xml");
    BorderImage* page = tabElement->tabBody_;
    page->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());

    page->SetImageRect(IntRect(48, 0, 64, 16));
    page->SetLayoutMode(LM_VERTICAL);
//    page->SetLayoutBorder(IntRect(10, 10, 10, 10));
    page->SetName("OptionPageGeneral");
    page->SetPivot(0, 0);
//    page->SetPosition(0, 0);
    page->SetWidth(400);
    page->SetStyleAuto();
    page->LoadXML(file->GetRoot());
    page->UpdateLayout();
//    tabElement->tabBody_->UpdateLayout();

    DropDownList* windowDropdown = dynamic_cast<DropDownList*>(page->GetChild("WindowDropdown", true));

    Text* result = new Text(context_);
    result->SetText("Windowed");
    result->SetStyle("DropDownItemEnumText");
    windowDropdown->AddItem(result);
    windowDropdown->AddItem(result);
    windowDropdown->AddItem(result);

}

