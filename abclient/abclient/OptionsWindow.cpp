#include "stdafx.h"
#include "OptionsWindow.h"
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
    Text* titleText = GetChildStaticCast<Text>("TitleText", true);
    titleText->SetText(scs->GetCaption(Events::E_SC_TOGGLEOPTIONS, "Options", true));
    SetBringToFront(true);
    SetBringToBack(true);

    UIElement* container = GetChild("Container", true);

    tabgroup_ = container->CreateChild<TabGroup>();
    tabgroup_->SetSize(container->GetSize());
    tabgroup_->SetPosition(0, 0);

    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_TOP);
    tabgroup_->SetColor(Color(0, 0, 0, 0));
    tabgroup_->SetStyleAuto();

    CreatePageGeneral(CreateTab(tabgroup_, "General"));
    CreatePageGraphics(CreateTab(tabgroup_, "Graphics"));
    CreatePageAudio(CreateTab(tabgroup_, "Audio"));
    CreatePageInput(CreateTab(tabgroup_, "Input"));
    CreatePageInterface(CreateTab(tabgroup_, "Interface"));

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
    Text* fovText = GetChildStaticCast<Text>("FovText", true);
    char buff[255] = { 0 };
    sprintf(buff, "FOV %d Deg", static_cast<int>(opt->GetCameraFov()));
    fovText->SetText(String(buff));
}

void OptionsWindow::SubscribeEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(OptionsWindow, HandleCloseClicked));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(OptionsWindow, HandleKeyDown));
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
    tabElement->tabBody_->SetLayoutMode(LM_VERTICAL);
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

    auto getWindowModeItem = [this](const String& text, WindowMode mode) -> Text*
    {
        Text* result = new Text(context_);
        result->SetText(text);
        result->SetStyle("DropDownItemEnumText");
        result->SetVar("Mode", static_cast<int>(mode));
        return result;
    };

    DropDownList* windowDropdown = wnd->GetChildStaticCast<DropDownList>("WindowDropdown", true);
    windowDropdown->AddItem(getWindowModeItem("Windowed", WindowMode::Windowed));
    windowDropdown->AddItem(getWindowModeItem("Fullscreen", WindowMode::Fullcreen));
    windowDropdown->AddItem(getWindowModeItem("Borderless", WindowMode::Borderless));
    if (opts->GetWindowMode() == WindowMode::Maximized)
        windowDropdown->SetSelection(0u);
    else
        windowDropdown->SetSelection(static_cast<unsigned>(opts->GetWindowMode()));

    SubscribeToEvent(windowDropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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

    CheckBox* shCheck = wnd->GetChildStaticCast<CheckBox>("StickCameraToHeadCheck", true);
    shCheck->SetChecked(opts->stickCameraToHead_);
    SubscribeToEvent(shCheck, E_TOGGLED, [this](StringHash, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->stickCameraToHead_ = checked;
    });
    CheckBox* mwCheck = wnd->GetChildStaticCast<CheckBox>("DisableMouseWalkingCheck", true);
    mwCheck->SetChecked(opts->disableMouseWalking_);
    SubscribeToEvent(mwCheck, E_TOGGLED, [this](StringHash, VariantMap& eventData)
    {
        using namespace Toggled;
        bool checked = eventData[P_STATE].GetBool();
        Options* opt = GetSubsystem<Options>();
        opt->disableMouseWalking_ = checked;
    });
    {
        Slider* slider = wnd->GetChildStaticCast<Slider>("MouseSensSlider", true);
        slider->SetValue(opts->mouseSensitivity_);
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
        {
            using namespace SliderChanged;
            float value = eventData[P_VALUE].GetFloat();
            Options* opt = GetSubsystem<Options>();
            opt->mouseSensitivity_ = value;
        });
    }
    CheckBox* mumbleCheck = wnd->GetChildStaticCast<CheckBox>("EnableMumbleCheck", true);
    mumbleCheck->SetChecked(opts->enableMumble_);
    SubscribeToEvent(mumbleCheck, E_TOGGLED, [this](StringHash, VariantMap& eventData)
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

        DropDownList* dropdown = wnd->GetChildStaticCast<DropDownList>("ShadowQualityDropdown", true);
        auto getItem = [this](const String& text, ShadowQuality qual) -> Text*
        {
            Text* result = new Text(context_);
            result->SetText(text);
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(qual));
            return result;
        };
        dropdown->AddItem(getItem("Simple 16Bit", SHADOWQUALITY_SIMPLE_16BIT));
        dropdown->AddItem(getItem("Simple 24Bit", SHADOWQUALITY_SIMPLE_24BIT));
        dropdown->AddItem(getItem("PCF 16Bit", SHADOWQUALITY_PCF_16BIT));
        dropdown->AddItem(getItem("PCF 24Bit", SHADOWQUALITY_PCF_24BIT));
        dropdown->AddItem(getItem("VSM", SHADOWQUALITY_VSM));
        dropdown->AddItem(getItem("Blur VSM", SHADOWQUALITY_BLUR_VSM));
        dropdown->SetSelection(static_cast<unsigned>(opts->GetShadowQuality()));
        SubscribeToEvent(dropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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
        DropDownList* materialDropdown = wnd->GetChildStaticCast<DropDownList>("MaterialQualityDropdown", true);
        auto getQualityItem = [this](const String& text, MaterialQuality mode) -> Text*
        {
            Text* result = new Text(context_);
            result->SetText(text);
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(mode));
            return result;
        };
        materialDropdown->AddItem(getQualityItem("Low", QUALITY_LOW));
        materialDropdown->AddItem(getQualityItem("Medium", QUALITY_MEDIUM));
        materialDropdown->AddItem(getQualityItem("High", QUALITY_HIGH));
        materialDropdown->AddItem(getQualityItem("Max", QUALITY_MAX));
        switch (opts->GetMaterialQuality())
        {
        case QUALITY_LOW:
            materialDropdown->SetSelection(0);
            break;
        case QUALITY_MEDIUM:
            materialDropdown->SetSelection(1);
            break;
        case QUALITY_HIGH:
            materialDropdown->SetSelection(2);
            break;
        case QUALITY_MAX:
            materialDropdown->SetSelection(3);
            break;
        default:
            assert(false);
        }
        SubscribeToEvent(materialDropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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

        DropDownList* textureQropdown = wnd->GetChildStaticCast<DropDownList>("TextureQualityDropdown", true);
        textureQropdown->AddItem(getQualityItem("Low", QUALITY_LOW));
        textureQropdown->AddItem(getQualityItem("Medium", QUALITY_MEDIUM));
        textureQropdown->AddItem(getQualityItem("High", QUALITY_HIGH));
        textureQropdown->AddItem(getQualityItem("Max", QUALITY_MAX));
        switch (opts->GetTextureQuality())
        {
        case QUALITY_LOW:
            textureQropdown->SetSelection(0);
            break;
        case QUALITY_MEDIUM:
            textureQropdown->SetSelection(1);
            break;
        case QUALITY_HIGH:
            textureQropdown->SetSelection(2);
            break;
        case QUALITY_MAX:
            textureQropdown->SetSelection(3);
            break;
        default:
            assert(false);
        }
        SubscribeToEvent(textureQropdown, E_ITEMSELECTED, [&](StringHash, VariantMap& eventData)
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
        DropDownList* dropdown = wnd->GetChildStaticCast<DropDownList>("TextureFilterDropdown", true);
        auto getItem = [this](const String& text, TextureFilterMode mode) -> Text*
        {
            Text* result = new Text(context_);
            result->SetText(text);
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(mode));
            return result;
        };
        dropdown->AddItem(getItem("Nearest", FILTER_NEAREST));
        dropdown->AddItem(getItem("Bilinear", FILTER_BILINEAR));
        dropdown->AddItem(getItem("Trilinear", FILTER_TRILINEAR));
        dropdown->AddItem(getItem("Anisotropic", FILTER_ANISOTROPIC));
        dropdown->AddItem(getItem("Nearest Anisotropic", FILTER_NEAREST_ANISOTROPIC));
        dropdown->AddItem(getItem("Default", FILTER_DEFAULT));
        dropdown->SetSelection(static_cast<unsigned>(opts->GetTextureFilterMode()));

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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
        CheckBox* check = wnd->GetChildStaticCast<CheckBox>("ShadowsCheck", true);
        check->SetChecked(opts->GetShadows());
        SubscribeToEvent(check, E_TOGGLED, [this](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetShadows(checked);
        });
    }
    {
        CheckBox* check = wnd->GetChildStaticCast<CheckBox>("VSynchCheck", true);
        check->SetChecked(opts->GetVSync());
        SubscribeToEvent(check, E_TOGGLED, [this](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetVSync(checked);
        });
    }

    {
        DropDownList* dropdown = wnd->GetChildStaticCast<DropDownList>("MaxFPSDropdown", true);
        auto getItem = [this](const String& text, int value) -> Text*
        {
            Text* result = new Text(context_);
            result->SetText(text);
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Value", value);
            return result;
        };
        dropdown->AddItem(getItem("Unlimited", 200));
        dropdown->AddItem(getItem("30", 30));
        dropdown->AddItem(getItem("40", 40));
        dropdown->AddItem(getItem("60", 60));
        dropdown->AddItem(getItem("120", 120));
        dropdown->AddItem(getItem("140", 140));
        dropdown->AddItem(getItem("144", 144));
        switch (GetSubsystem<Options>()->GetMaxFps())
        {
        case 200:
        case 0:
            dropdown->SetSelection(0);
            break;
        case 30:
            dropdown->SetSelection(1);
            break;
        case 40:
            dropdown->SetSelection(2);
            break;
        case 60:
            dropdown->SetSelection(3);
            break;
        case 120:
            dropdown->SetSelection(4);
            break;
        case 140:
            dropdown->SetSelection(5);
            break;
        case 144:
            dropdown->SetSelection(6);
            break;
        default:
            assert(false);
        }
        SubscribeToEvent(dropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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
        CheckBox* check = wnd->GetChildStaticCast<CheckBox>("SpecularLightningCheck", true);
        check->SetChecked(opts->GetSpecularLightning());
        SubscribeToEvent(check, E_TOGGLED, [this](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetSpecularLightning(checked);
        });
    }
    {
        Slider* slider = wnd->GetChildStaticCast<Slider>("FovSlider", true);
        slider->SetRange(MAX_FOV - MIN_FOV);
        slider->SetValue(opts->GetCameraFov() - MIN_FOV);
        {
            Text* fovText = wnd->GetChildStaticCast<Text>("FovText", true);
            char buff[255] = { 0 };
            sprintf(buff, "FOV %d Deg", static_cast<int>(opts->GetCameraFov()));
            fovText->SetText(String(buff));
        }
        SubscribeToEvent(slider, E_SLIDERCHANGED, URHO3D_HANDLER(OptionsWindow, HandleFovSliderChanged));
    }
    {
        CheckBox* check = wnd->GetChildStaticCast<CheckBox>("HighDPICheck", true);
        check->SetChecked(opts->GetHighDPI());
        SubscribeToEvent(check, E_TOGGLED, [this](StringHash, VariantMap& eventData)
        {
            using namespace Toggled;
            bool checked = eventData[P_STATE].GetBool();
            Options* opt = GetSubsystem<Options>();
            opt->SetHighDPI(checked);
        });
    }

    {
        DropDownList* dropdown = wnd->GetChildStaticCast<DropDownList>("AntiAliasingDropdown", true);
        auto getItem = [this](const String& text, AntiAliasingMode mode)
        {
            Text* result = new Text(context_);
            result->SetText(text);
            result->SetStyle("DropDownItemEnumText");
            result->SetVar("Mode", static_cast<unsigned>(mode));
            return result;
        };
        dropdown->AddItem(getItem("Node", AntiAliasingMode::None));
        dropdown->AddItem(getItem("FXAA3", AntiAliasingMode::FXAA3));
        dropdown->AddItem(getItem("MSAA x 2", AntiAliasingMode::MSAAx2));
        dropdown->AddItem(getItem("MSAA x 4", AntiAliasingMode::MSAAx4));
        dropdown->AddItem(getItem("MSAA x 8", AntiAliasingMode::MSAAx8));
        dropdown->AddItem(getItem("MSAA x 16", AntiAliasingMode::MSAAx16));
        dropdown->SetSelection(static_cast<unsigned>(opts->GetAntiAliasingMode()));

        SubscribeToEvent(dropdown, E_ITEMSELECTED, [this](StringHash, VariantMap& eventData)
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
        Slider* slider = wnd->GetChildStaticCast<Slider>("GainMasterSlider", true);
        slider->SetValue(opts->gainMaster_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
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
        Slider* slider = wnd->GetChildStaticCast<Slider>("GainEffectSlider", true);
        slider->SetValue(opts->gainEffect_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
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
        Slider* slider = wnd->GetChildStaticCast<Slider>("GainAmbientSlider", true);
        slider->SetValue(opts->gainAmbient_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
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
        Slider* slider = wnd->GetChildStaticCast<Slider>("GainVoiceSlider", true);
        slider->SetValue(opts->gainVoice_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
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
        Slider* slider = wnd->GetChildStaticCast<Slider>("GainMusicSlider", true);
        slider->SetValue(opts->gainMusic_ * slider->GetRange());
        SubscribeToEvent(slider, E_SLIDERCHANGED, [this](StringHash, VariantMap& eventData)
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
    ListView* lvw = GetChildStaticCast<ListView>("ShortcutsListView", true);
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
            txt->SetStyle("ListViewItemText");
            lvw->AddItem(txt);
        }
    }
    lvw->EnableLayoutUpdate();
    lvw->UpdateLayout();
}

void OptionsWindow::HandleShortcutItemSelected(StringHash, VariantMap& eventData)
{
    using namespace ItemSelected;
    ListView* lvw = static_cast<ListView*>(eventData[P_ELEMENT].GetPtr());
    Text* sel = dynamic_cast<Text*>(lvw->GetSelectedItem());
    if (sel)
    {
        StringHash _event = sel->GetVar("Event").GetStringHash();
        Button* addButton = GetChildStaticCast<Button>("AddButton", true);
        addButton->SetVar("Event", _event);
        ListView* hkLvw = GetChildStaticCast<ListView>("HotkeysListView", true);
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

void OptionsWindow::HandleKeyDown(StringHash, VariantMap& eventData)
{
    using namespace KeyDown;
    Key key = static_cast<Key>(eventData[P_KEY].GetInt());
    if (key == KEY_ESCAPE)
        if (IsVisible())
            SetVisible(false);
}

void OptionsWindow::CreatePageInput(TabElement* tabElement)
{
    BorderImage* page = tabElement->tabBody_;

    Window* wnd = page->CreateChild<Window>();
    LoadWindow(wnd, "UI/OptionPageInput.xml");
    wnd->SetPosition(0, 0);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    UIElement* hkContainer = GetChild("HotkeyEditContainer", true);
    HotkeyEdit* hkEdit = hkContainer->CreateChild<HotkeyEdit>("HotkeyEditor");
    hkEdit->SetLayoutBorder(IntRect(4, 4, 4, 4));
    hkEdit->SetAlignment(HA_CENTER, VA_BOTTOM);
    hkEdit->SetPosition(0, 0);
    hkEdit->SetMinSize(140, 30);
    hkEdit->SetSize(hkContainer->GetSize());
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    hkEdit->SetTexture(tex);
    hkEdit->SetImageRect(IntRect(48, 0, 64, 16));
    hkEdit->SetBorder(IntRect(4, 4, 4, 4));
    hkEdit->SetStyle("LineEdit");

    wnd->UpdateLayout();

    FillShortcutsList();
    ListView* lvw = GetChildStaticCast<ListView>("ShortcutsListView", true);
    SubscribeToEvent(lvw, E_ITEMSELECTED, URHO3D_HANDLER(OptionsWindow, HandleShortcutItemSelected));

    {
        Button* button = wnd->GetChildStaticCast<Button>("AddButton", true);
        SubscribeToEvent(button, E_RELEASED, [this](StringHash, VariantMap& eventData)
        {
            using namespace Released;

            HotkeyEdit* hkEdit = GetChildStaticCast<HotkeyEdit>("HotkeyEditor", true);
            if (hkEdit->Empty())
                return;

            Shortcut sc;
            Button* self = static_cast<Button*>(eventData[P_ELEMENT].GetPtr());
            StringHash _event = self->GetVar("Event").GetStringHash();
            sc.keyboardKey_ = hkEdit->GetKey();
            sc.modifiers_ = hkEdit->GetQualifiers();
            sc.mouseButton_ = hkEdit->GetMouseButton();
            Shortcuts* scs = GetSubsystem<Shortcuts>();
            unsigned id = scs->Add(_event, sc);

            if (id != 0)
            {
                ListView* lvwHk = GetChildStaticCast<ListView>("HotkeysListView", true);
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
        Button* button = wnd->GetChildStaticCast<Button>("DeleteButton", true);
        SubscribeToEvent(button, E_RELEASED, [this](StringHash, VariantMap&)
        {
            ListView* hkLvw = GetChildStaticCast<ListView>("HotkeysListView", true);
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
        Button* button = wnd->GetChildStaticCast<Button>("RestoreDefaultButton", true);
        SubscribeToEvent(button, E_RELEASED, [this](StringHash, VariantMap&)
        {
            Shortcuts* scs = GetSubsystem<Shortcuts>();
            scs->RestoreDefault();
            FillShortcutsList();
            ListView* hkLvw = GetChildStaticCast<ListView>("HotkeysListView", true);
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
    wnd->UpdateLayout();
}
