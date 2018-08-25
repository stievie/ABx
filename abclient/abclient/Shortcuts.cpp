#include "stdafx.h"
#include "Shortcuts.h"
#include "AbEvents.h"

const Shortcut Shortcut::EMPTY;

Shortcuts::Shortcuts(Context* context) :
    Object(context)
{
    Init();
    SubscribeToEvents();
}

Shortcuts::~Shortcuts()
{
    UnsubscribeFromAllEvents();
}

bool Shortcuts::Test(const StringHash& e)
{
    return triggered_[e];
}

void Shortcuts::Add(const Shortcut& sc)
{
    shortcuts_.Push(sc);
    shortcuts_.Back().id = shortcuts_.Size();
    triggered_[sc.event_] = false;
}

void Shortcuts::Init()
{
    Add({ AbEvents::E_SC_MOVEFORWARD, "Move forward", Trigger::None, SCANCODE_UNKNOWN, KEY_W });
    Add({ AbEvents::E_SC_MOVEFORWARD, "Move forward", Trigger::None, SCANCODE_UNKNOWN, KEY_UP });
    Add({ AbEvents::E_SC_MOVEBACKWARD, "Move backward", Trigger::None, SCANCODE_UNKNOWN, KEY_S });
    Add({ AbEvents::E_SC_MOVEBACKWARD, "Move backward", Trigger::None, SCANCODE_UNKNOWN, KEY_DOWN });
    Add({ AbEvents::E_SC_TURNLEFT, "Turn left", Trigger::None, SCANCODE_UNKNOWN, KEY_A });
    Add({ AbEvents::E_SC_TURNLEFT, "Turn left", Trigger::None, SCANCODE_UNKNOWN, KEY_LEFT });
    Add({ AbEvents::E_SC_TURNRIGHT, "Turn right", Trigger::None, SCANCODE_UNKNOWN, KEY_D });
    Add({ AbEvents::E_SC_TURNRIGHT, "Turn right", Trigger::None, SCANCODE_UNKNOWN, KEY_RIGHT });
    Add({ AbEvents::E_SC_MOVELEFT, "Move left", Trigger::None, SCANCODE_UNKNOWN, KEY_Q });
    Add({ AbEvents::E_SC_MOVERIGHT, "Move right", Trigger::None, SCANCODE_UNKNOWN, KEY_E });
    Add({ AbEvents::E_SC_AUTORUN, "Auto run", Trigger::Down, SCANCODE_UNKNOWN, KEY_R });

    Add({ AbEvents::E_SC_KEEPRUNNING, "Keep running", Trigger::Down, SCANCODE_UNKNOWN, KEY_R });
    Add({ AbEvents::E_SC_REVERSECAMERA, "Reverse Camera", Trigger::None, SCANCODE_UNKNOWN, KEY_Y });
    Add({ AbEvents::E_SC_HIGHLIGHTOBJECTS, "Highlight objects", Trigger::None, SCANCODE_UNKNOWN, KEY_LCTRL });

    Add({ AbEvents::E_SC_MOUSELOOK, "Mouse look", Trigger::None, SCANCODE_UNKNOWN, KEY_UNKNOWN, MOUSEB_RIGHT, 0, false });

    Add({ AbEvents::E_SC_DEFAULTACTION, "Attack/Interact", Trigger::Down, SCANCODE_UNKNOWN, KEY_SPACE });

    Add({ AbEvents::E_SC_TOGGLEMAP, "Map", Trigger::Down, SCANCODE_UNKNOWN, KEY_M });
    Add({ AbEvents::E_SC_TOGGLEPARTYWINDOW, "Party", Trigger::Down, SCANCODE_UNKNOWN, KEY_P });

    Add({ AbEvents::E_SC_TAKESCREENSHOT, "Take Screenshot", Trigger::Down, SCANCODE_UNKNOWN, KEY_PRINTSCREEN });

    Add({ AbEvents::E_SC_TOGGLEMAILWINDOW, "Mail", Trigger::Down });
    Add({ AbEvents::E_SC_LOGOUT, "Logout", Trigger::Down });
    Add({ AbEvents::E_SC_SELECTCHARACTER, "Select character", Trigger::Down });
    Add({ AbEvents::E_SC_TOGGLEOPTIONS, "Options", Trigger::Down, SCANCODE_UNKNOWN, KEY_F12 });
    Add({ AbEvents::E_SC_EXITPROGRAM, "Exit", Trigger::Down });

    // Chat Window
    Add({ AbEvents::E_SC_CHATGENERAL, "General", Trigger::Up, SCANCODE_UNKNOWN, KEY_1, MOUSEB_NONE, SC_MOD_SHIFT });
    Add({ AbEvents::E_SC_CHATGUILD, "Guild", Trigger::Up, SCANCODE_UNKNOWN, KEY_2, MOUSEB_NONE, SC_MOD_SHIFT });
    Add({ AbEvents::E_SC_CHATPARTY, "Party", Trigger::Up, SCANCODE_UNKNOWN, KEY_3, MOUSEB_NONE, SC_MOD_SHIFT });
    Add({ AbEvents::E_SC_CHATTRADE, "Trade", Trigger::Up, SCANCODE_UNKNOWN, KEY_4, MOUSEB_NONE, SC_MOD_SHIFT });
    Add({ AbEvents::E_SC_CHATWHISPER, "Whisper", Trigger::Up, SCANCODE_UNKNOWN, KEY_5, MOUSEB_NONE, SC_MOD_SHIFT });
    Add({ AbEvents::E_SC_TOGGLECHATWINDOW, "Whisper", Trigger::Down, SCANCODE_UNKNOWN, KEY_CARET });

    Add({ AbEvents::E_SC_HIDEUI, "Hide UI", Trigger::Down, SCANCODE_UNKNOWN, KEY_BACKSPACE });
}

void Shortcuts::Load(const XMLElement& root)
{
    XMLElement paramElem = root.GetChild("shortcut");
    if (!paramElem)
        return;

    while (paramElem)
    {
        unsigned id = paramElem.GetUInt("id");
        Shortcut sc;
        if (Get(id, sc) && sc.customizeable_)
        {
            // Only key can be customized to scan code
            sc.keyboardKey_ = static_cast<Key>(paramElem.GetUInt("key"));
            sc.mouseButton_ = static_cast<MouseButton>(paramElem.GetUInt("mousebutton"));
            sc.modifiers_ = paramElem.GetUInt("modifiers");
        }

        paramElem = paramElem.GetNext("shortcut");
    }
}

void Shortcuts::Save(XMLElement& root)
{
    for (const auto& sc : shortcuts_)
    {
        if (!sc.customizeable_)
            continue;

        XMLElement param = root.CreateChild("shortcut");
        param.SetUInt("id", sc.id);
        param.SetUInt("key", static_cast<unsigned>(sc.keyboardKey_));
        param.SetUInt("mousebutton", static_cast<unsigned>(sc.mouseButton_));
        param.SetUInt("modifiers", sc.modifiers_);
    }
}

const Shortcut& Shortcuts::Get(const StringHash& _event) const
{
    for (const auto& sc : shortcuts_)
        if (sc.event_ == _event)
            return sc;
    return Shortcut::EMPTY;
}

bool Shortcuts::Get(unsigned id, Shortcut& sc)
{
    for (const auto& _sc : shortcuts_)
        if (_sc.id == id)
        {
            sc = _sc;
            return true;
        }
    return false;
}

String Shortcuts::GetCaption(const StringHash& _event, const String& def /* = String::EMPTY */, unsigned align /* = 0 */)
{
    const Shortcut& sc = Get(_event);
    if (sc)
        return sc.Caption(align);
    return def;
}

void Shortcuts::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Shortcuts, HandleUpdate));
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(Shortcuts, HandleMouseDown));
    SubscribeToEvent(E_MOUSEBUTTONUP, URHO3D_HANDLER(Shortcuts, HandleMouseUp));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(Shortcuts, HandleKeyDown));
    SubscribeToEvent(E_KEYUP, URHO3D_HANDLER(Shortcuts, HandleKeyUp));
}

void Shortcuts::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    for (auto& t : triggered_)
        t.second_ = false;

    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();
    for (const auto& sc : shortcuts_)
    {
        if (triggered_[sc.event_])
            continue;
        triggered_[sc.event_] = (
            (sc.keyboardKey_ != KEY_UNKNOWN ? input->GetKeyDown(sc.keyboardKey_) : false) ||
            (sc.scanCode_ != SCANCODE_UNKNOWN ? input->GetScancodeDown(sc.scanCode_) : false) ||
            (sc.mouseButton_ != MOUSEB_NONE ? input->GetMouseButtonDown(sc.mouseButton_) : false)) &&
            (sc.modifiers_ == 0 || ModifiersMatch(sc.modifiers_));
    }
}

void Shortcuts::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyDown;
    bool repeat = eventData[P_REPEAT].GetBool();
    if (repeat)
        return;

    VariantMap& e = GetEventDataMap();
    int scanCode = eventData[P_SCANCODE].GetInt();
    int key = eventData[P_KEY].GetInt();
    for (const auto& sc : shortcuts_)
    {
        if (sc.keyboardKey_ == KEY_UNKNOWN && sc.scanCode_ == SCANCODE_UNKNOWN)
            continue;
        if (sc.trigger_ == Trigger::Down)
        {
            if (sc.keyboardKey_ != KEY_UNKNOWN && sc.keyboardKey_ != key)
                continue;
            if (sc.scanCode_ != SCANCODE_UNKNOWN && sc.scanCode_ != scanCode)
                continue;
            if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                // If we have modifiers also these this
                continue;
            SendEvent(sc.event_, e);
        }
    }
}

void Shortcuts::HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyUp;

    VariantMap& e = GetEventDataMap();
    int scanCode = eventData[P_SCANCODE].GetInt();
    int key = eventData[P_KEY].GetInt();
    for (const auto& sc : shortcuts_)
    {
        if (sc.keyboardKey_ == KEY_UNKNOWN && sc.scanCode_ == SCANCODE_UNKNOWN)
            continue;
        if (sc.trigger_ == Trigger::Up)
        {
            if (sc.keyboardKey_ != KEY_UNKNOWN && sc.keyboardKey_ != key)
                continue;
            if (sc.scanCode_ != SCANCODE_UNKNOWN && sc.scanCode_ != scanCode)
                continue;
            if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                // If we have modifiers also these this
                continue;
            SendEvent(sc.event_, e);
        }
    }
}

void Shortcuts::HandleMouseDown(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetUInt());

    VariantMap& e = GetEventDataMap();
    for (const auto& sc : shortcuts_)
    {
        if (sc.trigger_ == Trigger::Down)
        {
            if (sc.mouseButton_ == button)
            {
                if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                    // If we have modifiers also these this
                    continue;
                SendEvent(sc.event_, e);
            }
        }
    }
}

void Shortcuts::HandleMouseUp(StringHash eventType, VariantMap& eventData)
{
    using namespace MouseButtonUp;

    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetInt());

    VariantMap& e = GetEventDataMap();
    for (const auto& sc : shortcuts_)
    {
        if (sc.trigger_ == Trigger::Up)
        {
            if (sc.mouseButton_ == button)
            {
                if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                    // If we have modifiers also these this
                    continue;
                SendEvent(sc.event_, e);
            }
        }
    }
}

bool Shortcuts::ModifiersMatch(unsigned mods)
{
    Input* input = GetSubsystem<Input>();
    return (((mods & SC_MOD_CTRL) == 0) || input->GetKeyDown(KEY_LCTRL) || input->GetKeyDown(KEY_RCTRL)) &&
        (((mods & SC_MOD_LCTRL) == 0) || input->GetKeyDown(KEY_LCTRL)) &&
        (((mods & SC_MOD_RCTRL) == 0) || input->GetKeyDown(KEY_RCTRL)) &&

        (((mods & SC_MOD_SHIFT) == 0) || input->GetKeyDown(KEY_LSHIFT) || input->GetKeyDown(KEY_RSHIFT)) &&
        (((mods & SC_MOD_LSHIFT) == 0) || input->GetKeyDown(KEY_LSHIFT)) &&
        (((mods & SC_MOD_RSHIFT) == 0) || input->GetKeyDown(KEY_RSHIFT)) &&

        (((mods & SC_MOD_ALT) == 0) || input->GetKeyDown(KEY_LALT) || input->GetKeyDown(KEY_RALT)) &&
        (((mods & SC_MOD_LALT) == 0) || input->GetKeyDown(KEY_LALT)) &&
        (((mods & SC_MOD_RALT) == 0) || input->GetKeyDown(KEY_RALT));
}

