#include "stdafx.h"
#include "OptionsWindow.h"
#include "AbEvents.h"
#include "Options.h"
#include "HotkeyEdit.h"
#include "LevelManager.h"
#include "BaseLevel.h"
#include "Mumble.h"

void OptionsWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<OptionsWindow>();
}

OptionsWindow::OptionsWindow(Context* context) :
    Window(context)
{
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    LoadWindow(this, "UI/OptionsWindow.xml");
    SetMovable(true);
    SetName("OptionsWindow");

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* titleText = dynamic_cast<Text*>(GetChild("TitleText", true));
    titleText->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEOPTIONS, "Options", true));

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
    {
        TabElement* elem = CreateTab(tabgroup_, "Audio");
        CreatePageAudio(elem);
    }
    {
        TabElement* elem = CreateTab(tabgroup_, "Input");
        CreatePageInput(elem);
    }
    {
        TabElement* elem = CreateTab(tabgroup_, "Interface");
        CreatePageInterface(elem);
    }

    tabgroup_->SetEnabled(true);
    SubscribeToEvent(E_TABSELECTED, URHO3D_HANDLER(OptionsWindow, HandleTabSelected));

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

void OptionsWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void OptionsWindow::HandleTabSelected(StringHash, VariantMap&)
{
}

void OptionsWindow::HandleFovSliderChanged(StringHash, VariantMap& eventData)
{
    using namespace SliderChanged;
    float value = eventData[P_VALUE].GetFloat();
    Options* opt = GetSubsystem<Options>();
    opt->SetCameraFov(value + MIN_FOV);
    Text* fovText = dynamic_cast<Text*>(GetChild("FovText", true));
    char buff[255] = { 0 };
    sprintf(buff, "FOV %d Deg", static_cast<int>(opt->GetCameraFov()));
    fovText->SetText(String(buff));
}

void OptionsWindow::SubscribeEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(OptionsWindow, HandleCloseClicked));
}

TabElement* OptionsWindow::CreateTab(TabGroup* tabs, const String& page)
{
    static const IntVector2 tabSize(65, 20);
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
    if (opts->GetWindowMode() == WindowMode::Maximized)
        windowDropdown->SetSelection(0u);
    else
        windowDropdown->SetSelection(static_cast<unsigned>(opts->GetWindowMode()));

    SubscribeToEvent(windowDropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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
    SubscribeToEvent(shCheck, E_TOGGLED, [&](StringHash, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->stickCameraToHead_ = checked;
    });
    CheckBox* mwCheck = dynamic_cast<CheckBox*>(wnd->GetChild("DisableMouseWalkingCheck", true));
    mwCheck->SetChecked(opts->disableMouseWalking_);
    SubscribeToEvent(mwCheck, E_TOGGLED, [&](StringHash, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->disableMouseWalking_ = checked;
    });
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("MouseSensSlider", true));
        slider->SetValue(opts->mouseSensitivity_);
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Options* opt = GetSubsystem<Options>();
            opt->mouseSensitivity_ = value;
        });
    }
    CheckBox* mumbleCheck = dynamic_cast<CheckBox*>(wnd->GetChild("EnableMumbleCheck", true));
    mumbleCheck->SetChecked(opts->enableMumble_);
    SubscribeToEvent(mumbleCheck, E_TOGGLED, [&](StringHash, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        Mumble* mumble = GetSubsystem<Mumble>();
        opt->enableMumble_ = checked;
        if (checked)
        {
            if (!mumble->IsInitialized())
                mumble->Initialize();
        }
        else
        {
            if (mumble->IsInitialized())
                mumble->Shutdown();
        }
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
        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash, VariantMap& eventData)
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
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetVSync(checked);
        });
    }

    {
        unsigned selection = 0;
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("MaxFPSDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("Unlimited");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 200);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 200 || GetSubsystem<Options>()->GetMaxFps() == 0)
                selection = 0;
        }
        {
            Text* result = new Text(context_);
            result->SetText("30");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 30);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 30)
                selection = 1;
        }
        {
            Text* result = new Text(context_);
            result->SetText("40");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 40);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 40)
                selection = 2;
        }
        {
            Text* result = new Text(context_);
            result->SetText("60");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 60);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 60)
                selection = 3;
        }
        {
            Text* result = new Text(context_);
            result->SetText("120");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 120);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 120)
                selection = 4;
        }
        {
            Text* result = new Text(context_);
            result->SetText("140");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 140);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 140)
                selection = 5;
        }
        {
            Text* result = new Text(context_);
            result->SetText("144");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", 144);
            dropdown->AddItem(result);
            if (GetSubsystem<Options>()->GetMaxFps() == 144)
                selection = 6;
        }
        dropdown->SetSelection(selection);
        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                int value = static_cast<int>(elem->GetVar("Value").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetMaxFps(value);
            }
        });
    }

    {
        CheckBox* check = dynamic_cast<CheckBox*>(wnd->GetChild("SpecularLightningCheck", true));
        check->SetChecked(opts->GetSpecularLightning());
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetSpecularLightning(checked);
        });
    }
    {
        CheckBox* check = dynamic_cast<CheckBox*>(wnd->GetChild("HDRRenderingCheck", true));
        check->SetChecked(opts->GetHDRRendering());
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetHDRRendering(checked);
        });
    }
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("FovSlider", true));
        slider->SetRange(MAX_FOV - MIN_FOV);
        slider->SetValue(opts->GetCameraFov() - MIN_FOV);
        {
            Text* fovText = dynamic_cast<Text*>(wnd->GetChild("FovText", true));
            char buff[255] = { 0 };
            sprintf(buff, "FOV %d Deg", static_cast<int>(opts->GetCameraFov()));
            fovText->SetText(String(buff));
        }
        SubscribeToEvent(slider, E_SLIDERCHANGED, URHO3D_HANDLER(OptionsWindow, HandleFovSliderChanged));
    }
    {
        CheckBox* check = dynamic_cast<CheckBox*>(wnd->GetChild("HighDPICheck", true));
        check->SetChecked(opts->GetHighDPI());
        SubscribeToEvent(check, E_TOGGLED, [&](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetHighDPI(checked);
        });
    }

    {
        DropDownList* dropdown = dynamic_cast<DropDownList*>(wnd->GetChild("AntiAliasingDropdown", true));
        {
            Text* result = new Text(context_);
            result->SetText("None");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::None));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("FXAA3");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::FXAA3));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("MSAA x 2");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::MSAAx2));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("MSAA x 4");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::MSAAx4));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("MSAA x 8");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::MSAAx8));
            dropdown->AddItem(result);
        }
        {
            Text* result = new Text(context_);
            result->SetText("MSAA x 16");
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(AntiAliasingMode::MSAAx16));
            dropdown->AddItem(result);
        }
        dropdown->SetSelection(static_cast<unsigned>(opts->GetAntiAliasingMode()));

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
        {
            using namespace ItemSelected;
            unsigned sel = eventData[P_SELECTION].GetUInt();
            DropDownList* list = dynamic_cast<DropDownList*>(eventData[P_ELEMENT].GetPtr());
            UIElement* elem = list->GetItem(sel);
            if (elem)
            {
                AntiAliasingMode mode = static_cast<AntiAliasingMode>(elem->GetVar("Mode").GetUInt());
                Options* opt = GetSubsystem<Options>();
                opt->SetAntiAliasingMode(mode);
            }
        });
    }

}

void OptionsWindow::CreatePageAudio(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageAudio.xml");
    wnd->SetPosition(0, 0);
    wnd->SetWidth(390);
    wnd->UpdateLayout();

    Options* opts = GetSubsystem<Options>();

    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("GainMasterSlider", true));
        slider->SetValue(opts->gainMaster_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Slider* _s = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            Options* opt = GetSubsystem<Options>();
            opt->gainMaster_ = value / _s->GetRange();
            opt->UpdateAudio();
        });
    }
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("GainEffectSlider", true));
        slider->SetValue(opts->gainEffect_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Slider* _s = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            Options* opt = GetSubsystem<Options>();
            opt->gainEffect_ = value / _s->GetRange();
            opt->UpdateAudio();
        });
    }
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("GainAmbientSlider", true));
        slider->SetValue(opts->gainAmbient_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Slider* _s = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            Options* opt = GetSubsystem<Options>();
            opt->gainAmbient_ = value / _s->GetRange();
            opt->UpdateAudio();
        });
    }
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("GainVoiceSlider", true));
        slider->SetValue(opts->gainVoice_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Slider* _s = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            Options* opt = GetSubsystem<Options>();
            opt->gainVoice_ = value / _s->GetRange();
            opt->UpdateAudio();
        });
    }
    {
        Slider* slider = dynamic_cast<Slider*>(wnd->GetChild("GainMusicSlider", true));
        slider->SetValue(opts->gainMusic_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [&](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Slider* _s = static_cast<Slider*>(eventData[P_ELEMENT].GetPtr());
            Options* opt = GetSubsystem<Options>();
            opt->gainMusic_ = value / _s->GetRange();
            opt->UpdateAudio();
        });
    }
}

void OptionsWindow::FillShortcutsList()
{
    ListView* lvw = dynamic_cast<ListView*>(GetChild("ShortcutsListView", true));
    lvw->RemoveAllItems();
    Shortcuts* scs = GetSubsystem<Shortcuts>();
    for (const auto& sc : scs->shortcuts_)
    {
        if (sc.second_.customizeable_)
        {
            Text* txt = new Text(context_);
            txt->SetText(sc.second_.GetDescription());
            txt->SetMaxWidth(lvw->GetWidth());
            txt->SetWidth(lvw->GetWidth());
            txt->SetWordwrap(false);
            txt->SetVar("Event", sc.first_);
            txt->SetStyle("DropDownItemEnumText");
            lvw->AddItem(txt);
        }
    }
    lvw->EnableLayoutUpdate();
    lvw->UpdateLayout();
}

void OptionsWindow::HandleShortcutItemSelected(StringHash, VariantMap& eventData)
{
    using namespace ItemSelected;
    ListView* lvw = dynamic_cast<ListView*>(eventData[P_ELEMENT].GetPtr());
    Text* sel = dynamic_cast<Text*>(lvw->GetSelectedItem());
    if (sel)
    {
        StringHash _event = sel->GetVar("Event").GetStringHash();
        Button* addButton = dynamic_cast<Button*>(GetChild("AddButton", true));
        addButton->SetVar("Event", _event);
        ListView* hkLvw = dynamic_cast<ListView*>(GetChild("HotkeysListView", true));
        hkLvw->RemoveAllItems();
        Shortcuts* scs = GetSubsystem<Shortcuts>();
        const auto& sc = scs->shortcuts_[_event];
        for (const auto& s : sc.shortcuts_)
        {
            Text* txt = new Text(context_);
            txt->SetText(s.ShortcutNameLong(true));
            txt->SetMaxWidth(hkLvw->GetWidth());
            txt->SetWidth(hkLvw->GetWidth());
            txt->SetWordwrap(false);
            txt->SetVar("ID", s.id_);
            txt->SetStyle("DropDownItemEnumText");
            hkLvw->AddItem(txt);
        }
    }
}

void OptionsWindow::CreatePageInput(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageInput.xml");
    wnd->SetPosition(0, 0);
    wnd->SetWidth(390);
    wnd->SetHeight(page->GetHeight());

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UIElement* hkContainer = dynamic_cast<UIElement*>(GetChild("HotkeyEditContainer", true));
    HotkeyEdit* hkEdit = hkContainer->CreateChild<HotkeyEdit>("HotkeyEditor");
    hkEdit->SetLayoutBorder(IntRect(4, 4, 4, 4));
    hkEdit->SetPivot(0, 0);
    hkEdit->SetAlignment(HA_CENTER, VA_BOTTOM);
    hkEdit->SetStyleAuto();
    hkEdit->SetPosition(0, 0);
    hkEdit->SetMinSize(140, 30);
    hkEdit->SetSize(hkContainer->GetSize());
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    hkEdit->SetTexture(tex);
    hkEdit->SetImageRect(IntRect(48, 0, 64, 16));
    hkEdit->SetBorder(IntRect(4, 4, 4, 4));
    hkEdit->SetImageBorder(IntRect(0, 0, 0, 0));
    hkEdit->SetStyle("LineEdit");

    wnd->UpdateLayout();

    FillShortcutsList();
    ListView* lvw = dynamic_cast<ListView*>(GetChild("ShortcutsListView", true));
    SubscribeToEvent(lvw, E_ITEMSELECTED, URHO3D_HANDLER(OptionsWindow, HandleShortcutItemSelected));

    {
        Button* button = dynamic_cast<Button*>(wnd->GetChild("AddButton", true));
        SubscribeToEvent(button, E_RELEASED, [&](StringHash, VariantMap& eventData)
        {
            using namespace Released;

            HotkeyEdit* hkEdit = dynamic_cast<HotkeyEdit*>(GetChild("HotkeyEditor", true));
            if (hkEdit->Empty())
                return;

            Shortcut sc;
            Button* self = dynamic_cast<Button*>(eventData[P_ELEMENT].GetPtr());
            StringHash _event = self->GetVar("Event").GetStringHash();
            sc.keyboardKey_ = hkEdit->GetKey();
            sc.modifiers_ = hkEdit->GetQualifiers();
            sc.mouseButton_ = hkEdit->GetMouseButton();
            Shortcuts* scs = GetSubsystem<Shortcuts>();
            unsigned id = scs->Add(_event, sc);

            if (id != 0)
            {
                ListView* lvwHk = dynamic_cast<ListView*>(GetChild("HotkeysListView", true));
                Text* txt = new Text(context_);
                txt->SetText(sc.ShortcutName(true));
                txt->SetMaxWidth(lvwHk->GetWidth());
                txt->SetWidth(lvwHk->GetWidth());
                txt->SetWordwrap(false);
                txt->SetVar("ID", id);
                txt->SetStyle("DropDownItemEnumText");
                lvwHk->AddItem(txt);
            }
        });
    }
    {
        Button* button = dynamic_cast<Button*>(wnd->GetChild("DeleteButton", true));
        SubscribeToEvent(button, E_RELEASED, [&](StringHash, VariantMap&)
        {
            ListView* hkLvw = dynamic_cast<ListView*>(GetChild("HotkeysListView", true));
            Text* sel = dynamic_cast<Text*>(hkLvw->GetSelectedItem());
            if (sel)
            {
                unsigned id = sel->GetVar("ID").GetUInt();
                Shortcuts* scs = GetSubsystem<Shortcuts>();
                scs->Delete(id);
                hkLvw->RemoveItem(hkLvw->GetSelection());
            }
        });
    }

    {
        Button* button = dynamic_cast<Button*>(wnd->GetChild("RestoreDefaultButton", true));
        SubscribeToEvent(button, E_RELEASED, [this](StringHash, VariantMap&)
        {
            Shortcuts* scs = GetSubsystem<Shortcuts>();
            scs->RestoreDefault();
            FillShortcutsList();
            ListView* hkLvw = dynamic_cast<ListView*>(GetChild("HotkeysListView", true));
            hkLvw->RemoveAllItems();
        });
    }
}

void OptionsWindow::CreatePageInterface(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageInterface.xml");
    wnd->SetPosition(0, 0);
    wnd->SetWidth(390);
    wnd->SetHeight(page->GetHeight());
    wnd->UpdateLayout();
}
