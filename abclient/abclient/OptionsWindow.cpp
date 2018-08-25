#include "stdafx.h"
#include "OptionsWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "Options.h"

void OptionsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<OptionsWindow>();
}

OptionsWindow::OptionsWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    LoadWindow(this, "UI/OptionsWindow.xml");
    SetMovable(true);
    SetName("OptionsWindow");

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* titleText = dynamic_cast<Text*>(GetChild("TitleText", true));
    titleText->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEOPTIONS, "Options"));

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
    {
        TabElement* elem = CreateTab(tabgroup_, "Graphics");
        CreatePageGraphics(elem);
    }
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

void OptionsWindow::LoadWindow(Window* wnd, const String& fileName)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* xml = cache->GetResource<XMLFile>(fileName);
    wnd->LoadXML(xml->GetRoot());
    // It seems this isn't loaded from the XML file
    wnd->SetLayoutMode(LM_VERTICAL);
    wnd->SetLayoutBorder(IntRect(4, 4, 4, 4));
    wnd->SetPivot(0, 0);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    wnd->SetTexture(tex);
    wnd->SetImageRect(IntRect(48, 0, 64, 16));
    wnd->SetBorder(IntRect(4, 4, 4, 4));
    wnd->SetImageBorder(IntRect(0, 0, 0, 0));
    wnd->SetResizeBorder(IntRect(8, 8, 8, 8));
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

    tabElement->tabBody_->SetImageRect(IntRect(48, 0, 64, 16));
    tabElement->tabBody_->SetLayoutMode(LM_FREE);
    tabElement->tabBody_->SetPivot(0, 0);
    tabElement->tabBody_->SetPosition(0, 0);
    tabElement->tabBody_->SetStyleAuto();
    return tabElement;
}

void OptionsWindow::CreatePageGeneral(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageGeneral.xml");
    wnd->SetPosition(0, 0);
    wnd->SetWidth(390);
    wnd->UpdateLayout();

    Options* opts = GetSubsystem<Options>();

    DropDownList* windowDropdown = dynamic_cast<DropDownList*>(wnd->GetChild("WindowDropdown", true));

    {
        Text* result = new Text(context_);
        result->SetText("Windowed");
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Mode", static_cast<int>(WindowMode::Windowed));
        windowDropdown->AddItem(result);
    }
    {
        Text* result = new Text(context_);
        result->SetText("Fullscreen");
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Mode", static_cast<int>(WindowMode::Fullcreeen));
        windowDropdown->AddItem(result);
    }
    {
        Text* result = new Text(context_);
        result->SetText("Borderless");
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Mode", static_cast<unsigned>(WindowMode::Borderless));
        windowDropdown->AddItem(result);
    }
    windowDropdown->SetSelection(static_cast<unsigned>(opts->GetWindowMode()));

    SubscribeToEvent(windowDropdown, E_ITEMSELECTED, [&](StringHash eventType, VariantMap& eventData)
    {
        using namespace ItemSelected;
        unsigned sel = eventData[P_SELECTION].GetUInt();
        DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
        UIElement* elem = list->GetItem(sel);
        if (elem)
        {
            WindowMode mode = static_cast<WindowMode>(elem->GetVar("Mode").GetUInt());
            Options* opt = GetSubsystem<Options>();
            opt->SetWindowMode(mode);
        }
    });

    CheckBox* shCheck = dynamic_cast<CheckBox*>(wnd->GetChild("StickCameraToHeadCheck", true));
    shCheck->SetChecked(opts->stickCameraToHead_);
    SubscribeToEvent(shCheck, E_TOGGLED, [&](StringHash eventType, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->stickCameraToHead_ = checked;
    });
    CheckBox* mwCheck = dynamic_cast<CheckBox*>(wnd->GetChild("DisableMouseWalkingCheck", true));
    mwCheck->SetChecked(opts->disableMouseWalking_);
    SubscribeToEvent(mwCheck, E_TOGGLED, [&](StringHash eventType, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->disableMouseWalking_ = checked;
    });
}

void OptionsWindow::CreatePageGraphics(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageGraphics.xml");
    wnd->SetPosition(0, 0);
    wnd->SetWidth(390);
    wnd->UpdateLayout();

    Options* opts = GetSubsystem<Options>();

    {
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("ShadowQualityDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("Simple 16Bit");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_SIMPLE_16BIT));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Simple 24Bit");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_SIMPLE_24BIT));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("PCF 16Bit");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_PCF_16BIT));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("PCF 24Bit");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_PCF_24BIT));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("VSM");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_VSM));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Blur VSM");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(SHADOWQUALITY_BLUR_VSM));
            dropdown->AddItem(result);
        }
        dropdown->SetSelection(static_cast<unsigned>(opts->GetShadowQuality()));
        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                ShadowQuality mode = static_cast<ShadowQuality>(elem->GetVar("Mode").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetShadowQuality(mode);
            }
        });
    }

    {
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("MaterialQualityDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("Low");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_LOW));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_LOW)
                dropdown->SetSelection(0);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Medium");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_MEDIUM));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_MEDIUM)
                dropdown->SetSelection(1);
        }
        {
            Text* result = new Text(context_);
            result->SetText("High");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_HIGH));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_HIGH)
                dropdown->SetSelection(2);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Max");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_MAX));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_MAX)
                dropdown->SetSelection(3);
        }

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                MaterialQuality mode = static_cast<MaterialQuality>(elem->GetVar("Mode").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetMaterialQuality(mode);
            }
        });
    }

    {
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("TextureQualityDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("Low");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_LOW));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_LOW)
                dropdown->SetSelection(0);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Medium");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_MEDIUM));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_MEDIUM)
                dropdown->SetSelection(1);
        }
        {
            Text* result = new Text(context_);
            result->SetText("High");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_HIGH));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_HIGH)
                dropdown->SetSelection(2);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Max");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(QUALITY_MAX));
            dropdown->AddItem(result);
            if (opts->GetMaterialQuality() == QUALITY_MAX)
                dropdown->SetSelection(3);
        }

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                MaterialQuality mode = static_cast<MaterialQuality>(elem->GetVar("Mode").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetTextureQuality(mode);
            }
        });
    }

    {
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("TextureFilterDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("Nearest");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_NEAREST));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Bilinear");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_BILINEAR));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Trilinear");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_TRILINEAR));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Anisotropic");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_ANISOTROPIC));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Nearest Anisotropic");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_NEAREST_ANISOTROPIC));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("Default");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(FILTER_DEFAULT));
            dropdown->AddItem(result);
        }
        dropdown->SetSelection(static_cast<unsigned>(opts->GetTextureFilterMode()));

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                TextureFilterMode mode = static_cast<TextureFilterMode>(elem->GetVar("Mode").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetTextureFilterMode(mode);
            }
        });
    }

    {
        CheckBox* check = dynamic_cast<CheckBox*>(wnd->GetChild("ShadowsCheck", true));
        check->SetChecked(opts->GetShadows());
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetShadows(checked);
        });
    }
    {
        CheckBox* check = dynamic_cast<CheckBox*>(wnd->GetChild("VSynchCheck", true));
        check->SetChecked(opts->GetVSync());
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash eventType, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetVSync(checked);
        });
    }

}

