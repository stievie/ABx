#include "stdafx.h"
#include "Shortcuts.h"
#include "AbEvents.h"

Shortcuts::Shortcuts(Context* context) :
    Object(context)
{
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
    if (sc.keyboardKey_ == KEY_UNKNOWN && sc.scanCode_ == SCANCODE_UNKNOWN && sc.mouseButton_ == MOUSEB_NONE)
        return;

    shortcuts_.Push(sc);
    triggered_[sc.event_] = false;
}

void Shortcuts::AddDefault()
{
    Add({ "Move forward", Trigger::None, AbEvents::E_SC_MOVEFORWARD, SCANCODE_UNKNOWN, KEY_W });
    Add({ "Move forward", Trigger::None, AbEvents::E_SC_MOVEFORWARD, SCANCODE_UNKNOWN, KEY_UP });
    Add({ "Move backward", Trigger::None, AbEvents::E_SC_MOVEBACKWARD, SCANCODE_UNKNOWN, KEY_S });
    Add({ "Move backward", Trigger::None, AbEvents::E_SC_MOVEBACKWARD, SCANCODE_UNKNOWN, KEY_DOWN });
    Add({ "Turn left", Trigger::None, AbEvents::E_SC_TURNLEFT, SCANCODE_UNKNOWN, KEY_A });
    Add({ "Turn left", Trigger::None, AbEvents::E_SC_TURNLEFT, SCANCODE_UNKNOWN, KEY_LEFT });
    Add({ "Turn right", Trigger::None, AbEvents::E_SC_TURNRIGHT, SCANCODE_UNKNOWN, KEY_D });
    Add({ "Turn right", Trigger::None, AbEvents::E_SC_TURNRIGHT, SCANCODE_UNKNOWN, KEY_RIGHT });
    Add({ "Move left", Trigger::None, AbEvents::E_SC_MOVELEFT, SCANCODE_UNKNOWN, KEY_Q });
    Add({ "Move right", Trigger::None, AbEvents::E_SC_MOVERIGHT, SCANCODE_UNKNOWN, KEY_E });

    Add({ "Keep running", Trigger::Down, AbEvents::E_SC_KEEPRUNNING, SCANCODE_UNKNOWN, KEY_R });
    Add({ "Reverse Camera", Trigger::None, AbEvents::E_SC_REVERSECAMERA, SCANCODE_UNKNOWN, KEY_Y });
    Add({ "Goto selected", Trigger::Down, AbEvents::E_SC_REVERSECAMERA, SCANCODE_UNKNOWN, KEY_SPACE });
    Add({ "Highlight objects", Trigger::None, AbEvents::E_SC_HIGHLIGHTOBJECTS, SCANCODE_LCTRL, KEY_UNKNOWN });

    Add({ "Mouse look", Trigger::None, AbEvents::E_SC_MOUSELOOK, SCANCODE_UNKNOWN, KEY_UNKNOWN, MOUSEB_RIGHT });
}

void Shortcuts::Load(const XMLElement& root)
{
    XMLElement paramElem = root.GetChild("shortcut");
    if (!paramElem)
    {
        AddDefault();
        return;
    }

    while (paramElem)
    {
        Shortcut sc;
        sc.name_ = paramElem.GetAttribute("name");
        sc.trigger_ = static_cast<Trigger>(paramElem.GetUInt("trigger"));
        sc.event_ = StringHash(paramElem.GetUInt("event"));
        sc.scanCode_ = static_cast<Scancode>(paramElem.GetUInt("scancode"));
        sc.keyboardKey_ = static_cast<Key>(paramElem.GetUInt("key"));
        sc.mouseButton_ = static_cast<MouseButton>(paramElem.GetUInt("mousebutton"));
        sc.modifiers_ = paramElem.GetUInt("modifiers");
        Add(sc);

        paramElem = paramElem.GetNext("shortcut");
    }
}

void Shortcuts::Save(XMLElement& root)
{
    for (const auto& sc : shortcuts_)
    {
        XMLElement param = root.CreateChild("shortcut");
        param.SetString("name", sc.name_);
        param.SetUInt("trigger", static_cast<unsigned>(sc.trigger_));
        param.SetUInt("event", sc.event_.Value());
        param.SetUInt("scancode", static_cast<unsigned>(sc.scanCode_));
        param.SetUInt("key", static_cast<unsigned>(sc.keyboardKey_));
        param.SetUInt("mousebutton", static_cast<unsigned>(sc.mouseButton_));
        param.SetUInt("modifiers", sc.modifiers_);
    }
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
    int qualifiers = eventData[P_QUALIFIERS].GetInt();
    for (const auto& sc : shortcuts_)
    {
        if (sc.trigger_ == Trigger::Down)
        {
            if (sc.scanCode_ == scanCode || sc.keyboardKey_ == key)
            {
                if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                    // If we have modifiers also these this
                    continue;
                SendEvent(sc.event_, e);
            }
        }
    }
}

void Shortcuts::HandleKeyUp(StringHash eventType, VariantMap& eventData)
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
    int qualifiers = eventData[P_QUALIFIERS].GetInt();
    for (const auto& sc : shortcuts_)
    {
        if (sc.trigger_ == Trigger::Up)
        {
            if (sc.scanCode_ == scanCode || sc.keyboardKey_ == key)
            {
                if (sc.modifiers_ != 0 && !ModifiersMatch(sc.modifiers_))
                    // If we have modifiers also these this
                    continue;
                SendEvent(sc.event_, e);
            }
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
    return (((mods & SC_MOD_CTRL) == 0) || input->GetKeyPress(KEY_LCTRL) || input->GetKeyPress(KEY_RCTRL)) &&
        (((mods & SC_MOD_LCTRL) == 0) || input->GetKeyPress(KEY_LCTRL)) &&
        (((mods & SC_MOD_RCTRL) == 0) || input->GetKeyPress(KEY_RCTRL)) &&

        (((mods & SC_MOD_SHIFT) == 0) || input->GetKeyPress(KEY_LSHIFT) || input->GetKeyPress(KEY_RSHIFT)) &&
        (((mods & SC_MOD_LSHIFT) == 0) || input->GetKeyPress(KEY_LSHIFT)) &&
        (((mods & SC_MOD_RSHIFT) == 0) || input->GetKeyPress(KEY_RSHIFT)) &&

        (((mods & SC_MOD_ALT) == 0) || input->GetKeyPress(KEY_LALT) || input->GetKeyPress(KEY_RALT)) &&
        (((mods & SC_MOD_LALT) == 0) || input->GetKeyPress(KEY_LALT)) &&
        (((mods & SC_MOD_RALT) == 0) || input->GetKeyPress(KEY_RALT));
}

