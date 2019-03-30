#include "stdafx.h"

#include "Shortcuts.h"
#include "AbEvents.h"
#include <Urho3D/Container/Sort.h>

const Shortcut Shortcut::EMPTY;
unsigned Shortcuts::shortcutIds = 0;

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

unsigned Shortcuts::Add(const StringHash& _event, const Shortcut& sc)
{
    for (const auto& s : shortcuts_[_event].shortcuts_)
    {
        if (s == sc)
            // Can not assign the same shortcut to the same event
            return 0;
    }

    unsigned id = ++shortcutIds;
    Shortcut _sc(sc);
    _sc.id_ = id;
    shortcuts_[_event].shortcuts_.Push(_sc);
    triggered_[_event] = false;
    return id;
}

void Shortcuts::Init()
{
    shortcuts_.Clear();
    // Exit
    shortcuts_[AbEvents::E_SC_EXITPROGRAM] = ShortcutEvent(AbEvents::E_SC_EXITPROGRAM, "Exit", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SHOWCREDITS] = ShortcutEvent(AbEvents::E_SC_LOGOUT, "Credits", Trigger::Down, String::EMPTY, false);
    shortcuts_[AbEvents::E_SC_TOGGLEMUTEAUDIO] = ShortcutEvent(AbEvents::E_SC_TOGGLEMUTEAUDIO, "Mute", Trigger::Down, "Mute Audio");
    shortcuts_[AbEvents::E_SC_LOGOUT] = ShortcutEvent(AbEvents::E_SC_LOGOUT, "Logout", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SELECTCHARACTER] = ShortcutEvent(AbEvents::E_SC_SELECTCHARACTER, "Select character", Trigger::Down);

    // Move
    shortcuts_[AbEvents::E_SC_MOVEFORWARD] = ShortcutEvent(AbEvents::E_SC_MOVEFORWARD, "Move forward", Trigger::None);
    shortcuts_[AbEvents::E_SC_MOVEBACKWARD] = ShortcutEvent(AbEvents::E_SC_MOVEBACKWARD, "Move backward", Trigger::None);
    shortcuts_[AbEvents::E_SC_TURNLEFT] = ShortcutEvent(AbEvents::E_SC_TURNLEFT, "Turn left", Trigger::None);
    shortcuts_[AbEvents::E_SC_TURNRIGHT] = ShortcutEvent(AbEvents::E_SC_TURNRIGHT, "Turn right", Trigger::None);
    shortcuts_[AbEvents::E_SC_MOVELEFT] = ShortcutEvent(AbEvents::E_SC_MOVELEFT, "Move left", Trigger::None);
    shortcuts_[AbEvents::E_SC_MOVERIGHT] = ShortcutEvent(AbEvents::E_SC_MOVERIGHT, "Move right", Trigger::None);
    shortcuts_[AbEvents::E_SC_KEEPRUNNING] = ShortcutEvent(AbEvents::E_SC_KEEPRUNNING, "Keep running", Trigger::Down);

    shortcuts_[AbEvents::E_SC_REVERSECAMERA] = ShortcutEvent(AbEvents::E_SC_REVERSECAMERA, "Reverse Camera", Trigger::None);
    shortcuts_[AbEvents::E_SC_HIGHLIGHTOBJECTS] = ShortcutEvent(AbEvents::E_SC_HIGHLIGHTOBJECTS, "Highlight objects", Trigger::None);
    shortcuts_[AbEvents::E_SC_MOUSELOOK] = ShortcutEvent(AbEvents::E_SC_MOUSELOOK, "Mouse look", Trigger::None, String::EMPTY, false);
    shortcuts_[AbEvents::E_SC_DEFAULTACTION] = ShortcutEvent(AbEvents::E_SC_DEFAULTACTION, "Attack/Interact", Trigger::Down);

    // Skill
    shortcuts_[AbEvents::E_SC_USESKILL1] = ShortcutEvent(AbEvents::E_SC_USESKILL1, "Use Skill 1", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL2] = ShortcutEvent(AbEvents::E_SC_USESKILL2, "Use Skill 2", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL3] = ShortcutEvent(AbEvents::E_SC_USESKILL3, "Use Skill 3", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL4] = ShortcutEvent(AbEvents::E_SC_USESKILL4, "Use Skill 4", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL5] = ShortcutEvent(AbEvents::E_SC_USESKILL5, "Use Skill 5", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL6] = ShortcutEvent(AbEvents::E_SC_USESKILL6, "Use Skill 6", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL7] = ShortcutEvent(AbEvents::E_SC_USESKILL7, "Use Skill 7", Trigger::Down);
    shortcuts_[AbEvents::E_SC_USESKILL8] = ShortcutEvent(AbEvents::E_SC_USESKILL8, "Use Skill 8", Trigger::Down);

    shortcuts_[AbEvents::E_SC_WEAPONSET1] = ShortcutEvent(AbEvents::E_SC_WEAPONSET1, "Weapon Set 1", Trigger::Down);
    shortcuts_[AbEvents::E_SC_WEAPONSET2] = ShortcutEvent(AbEvents::E_SC_WEAPONSET2, "Weapon Set 2", Trigger::Down);
    shortcuts_[AbEvents::E_SC_WEAPONSET3] = ShortcutEvent(AbEvents::E_SC_WEAPONSET3, "Weapon Set 3", Trigger::Down);
    shortcuts_[AbEvents::E_SC_WEAPONSET4] = ShortcutEvent(AbEvents::E_SC_WEAPONSET4, "Weapon Set 4", Trigger::Down);

    shortcuts_[AbEvents::E_SC_CANCEL] = ShortcutEvent(AbEvents::E_SC_CANCEL, "Cancel skill/attack", Trigger::Down);

    // Select
    shortcuts_[AbEvents::E_SC_SELECTSELF] = ShortcutEvent(AbEvents::E_SC_SELECTSELF, "Select Self", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SELECTNEXTFOE] = ShortcutEvent(AbEvents::E_SC_SELECTNEXTFOE, "Select next Foe", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SELECTPREVFOE] = ShortcutEvent(AbEvents::E_SC_SELECTPREVFOE, "Select previous Foe", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SELECTNEXTALLY] = ShortcutEvent(AbEvents::E_SC_SELECTNEXTALLY, "Select next Ally", Trigger::Down);
    shortcuts_[AbEvents::E_SC_SELECTPREVALLY] = ShortcutEvent(AbEvents::E_SC_SELECTPREVALLY, "Select previous Ally", Trigger::Down);

    // UI
    shortcuts_[AbEvents::E_SC_HIDEUI] = ShortcutEvent(AbEvents::E_SC_HIDEUI, "Hide UI", Trigger::Down);
    shortcuts_[AbEvents::E_SC_TOGGLEMAP] = ShortcutEvent(AbEvents::E_SC_TOGGLEMAP, "Map", Trigger::Down, "Toggle Map window");
    shortcuts_[AbEvents::E_SC_TOGGLEPARTYWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLEPARTYWINDOW, "Party", Trigger::Down, "Toggle Party window");
    shortcuts_[AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, "Friends", Trigger::Down, "Toggle Friendlist window");
    shortcuts_[AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW, "Mission Map", Trigger::Down, "Toggle Mission map");
    shortcuts_[AbEvents::E_SC_TAKESCREENSHOT] = ShortcutEvent(AbEvents::E_SC_TAKESCREENSHOT, "Take Screenshot", Trigger::Down);
    shortcuts_[AbEvents::E_SC_TOGGLEMAILWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLEMAILWINDOW, "Mail", Trigger::Down, "Toggle Mail window");
    shortcuts_[AbEvents::E_SC_TOGGLENEWMAILWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLENEWMAILWINDOW, "New Mail", Trigger::Down, "Show New Mail window");
    shortcuts_[AbEvents::E_SC_TOGGLEOPTIONS] = ShortcutEvent(AbEvents::E_SC_TOGGLEOPTIONS, "Options", Trigger::Down, "Show Options window");
    // Chat
    shortcuts_[AbEvents::E_SC_TOGGLECHATWINDOW] = ShortcutEvent(AbEvents::E_SC_TOGGLECHATWINDOW, "Chat", Trigger::Down, "Toggle Chat window");
    shortcuts_[AbEvents::E_SC_CHATGENERAL] = ShortcutEvent(AbEvents::E_SC_CHATGENERAL, "General", Trigger::Up, "General chat");
    shortcuts_[AbEvents::E_SC_CHATGUILD] = ShortcutEvent(AbEvents::E_SC_CHATGUILD, "Guild", Trigger::Up, "Guild chat");
    shortcuts_[AbEvents::E_SC_CHATPARTY] = ShortcutEvent(AbEvents::E_SC_CHATPARTY, "Party", Trigger::Up, "Party chat");
    shortcuts_[AbEvents::E_SC_CHATTRADE] = ShortcutEvent(AbEvents::E_SC_CHATTRADE, "Trade", Trigger::Up, "Trade chat");
    shortcuts_[AbEvents::E_SC_CHATWHISPER] = ShortcutEvent(AbEvents::E_SC_CHATWHISPER, "Whisper", Trigger::Up, "Whisper chat");

#ifdef DEBUG_HUD
    shortcuts_[AbEvents::E_SC_TOGGLEDEBUGHUD] = ShortcutEvent(AbEvents::E_SC_TOGGLEDEBUGHUD, "Debug HUD", Trigger::Down, "Toggle Debug HUD");
    shortcuts_[AbEvents::E_SC_TOGGLECONSOLE] = ShortcutEvent(AbEvents::E_SC_TOGGLECONSOLE, "Console", Trigger::Down, "Toggle Console");
#endif

    // Add non customizable shortcuts
    Add(AbEvents::E_SC_MOUSELOOK, { KEY_UNKNOWN, MOUSEB_RIGHT });
}

void Shortcuts::AddDefault()
{
    Add(AbEvents::E_SC_MOVEFORWARD, { KEY_W });
    Add(AbEvents::E_SC_MOVEFORWARD, { KEY_UP });
    Add(AbEvents::E_SC_MOVEBACKWARD, { KEY_S });
    Add(AbEvents::E_SC_MOVEBACKWARD, { KEY_DOWN });
    Add(AbEvents::E_SC_TURNLEFT, { KEY_A });
    Add(AbEvents::E_SC_TURNLEFT, { KEY_LEFT });
    Add(AbEvents::E_SC_TURNRIGHT, { KEY_D });
    Add(AbEvents::E_SC_TURNRIGHT, { KEY_RIGHT });
    Add(AbEvents::E_SC_MOVELEFT, { KEY_Q });
    Add(AbEvents::E_SC_MOVERIGHT, { KEY_E });

    Add(AbEvents::E_SC_KEEPRUNNING, { KEY_R });
    Add(AbEvents::E_SC_REVERSECAMERA, { KEY_Y });
    Add(AbEvents::E_SC_HIGHLIGHTOBJECTS, { KEY_LCTRL });

    Add(AbEvents::E_SC_DEFAULTACTION, { KEY_SPACE });
    Add(AbEvents::E_SC_TOGGLEMAP, { KEY_M });
    Add(AbEvents::E_SC_TOGGLEPARTYWINDOW, { KEY_P });
    Add(AbEvents::E_SC_TOGGLEFRIENDLISTWINDOW, { KEY_N });
    Add(AbEvents::E_SC_TAKESCREENSHOT, { KEY_PRINTSCREEN });

    Add(AbEvents::E_SC_TOGGLEOPTIONS, { KEY_F11 });

    Add(AbEvents::E_SC_CHATGENERAL, { KEY_1, MOUSEB_NONE, SC_MOD_SHIFT });
    Add(AbEvents::E_SC_CHATGUILD, { KEY_2, MOUSEB_NONE, SC_MOD_SHIFT });
    Add(AbEvents::E_SC_CHATPARTY, { KEY_3, MOUSEB_NONE, SC_MOD_SHIFT });
    Add(AbEvents::E_SC_CHATTRADE, { KEY_4, MOUSEB_NONE, SC_MOD_SHIFT });
    Add(AbEvents::E_SC_CHATWHISPER, { KEY_5, MOUSEB_NONE, SC_MOD_SHIFT });
    Add(AbEvents::E_SC_TOGGLECHATWINDOW, { KEY_CARET });

    Add(AbEvents::E_SC_HIDEUI, { KEY_BACKSPACE });

    Add(AbEvents::E_SC_USESKILL1, { KEY_1 });
    Add(AbEvents::E_SC_USESKILL2, { KEY_2 });
    Add(AbEvents::E_SC_USESKILL3, { KEY_3 });
    Add(AbEvents::E_SC_USESKILL4, { KEY_4 });
    Add(AbEvents::E_SC_USESKILL5, { KEY_5 });
    Add(AbEvents::E_SC_USESKILL6, { KEY_6 });
    Add(AbEvents::E_SC_USESKILL7, { KEY_7 });
    Add(AbEvents::E_SC_USESKILL8, { KEY_8 });
    Add(AbEvents::E_SC_WEAPONSET1, { KEY_F1 });
    Add(AbEvents::E_SC_WEAPONSET2, { KEY_F2 });
    Add(AbEvents::E_SC_WEAPONSET3, { KEY_F3 });
    Add(AbEvents::E_SC_WEAPONSET4, { KEY_F4 });

    Add(AbEvents::E_SC_CANCEL, { KEY_ESCAPE });

    Add(AbEvents::E_SC_SELECTSELF, { KEY_F });
    Add(AbEvents::E_SC_SELECTNEXTFOE, { KEY_TAB });
    Add(AbEvents::E_SC_SELECTPREVFOE, { KEY_TAB, MOUSEB_NONE, SC_MOD_SHIFT });

    Add(AbEvents::E_SC_TOGGLEMISSIONMAPWINDOW, { KEY_U });
#ifdef DEBUG_HUD
    Add(AbEvents::E_SC_TOGGLEDEBUGHUD, { KEY_F12 });
    Add(AbEvents::E_SC_TOGGLECONSOLE, { KEY_F9 });
#endif
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
        StringHash _event = StringHash(paramElem.GetUInt("event"));
        ShortcutEvent& scEvent = shortcuts_[_event];
        if (scEvent.customizeable_)
        {
            Shortcut sc{ static_cast<Key>(paramElem.GetUInt("key")),
                static_cast<MouseButton>(paramElem.GetUInt("mousebutton")),
                paramElem.GetUInt("modifiers") };
            Add(_event, sc);
        }

        paramElem = paramElem.GetNext("shortcut");
    }

}

void Shortcuts::Save(XMLElement& root)
{
    root.RemoveChildren("shortcut");
    for (const auto& sc : shortcuts_)
    {
        if (!sc.second_.customizeable_)
            continue;

        for (const auto& s : sc.second_.shortcuts_)
        {
            XMLElement param = root.CreateChild("shortcut");
            param.SetUInt("event", sc.first_.Value());
            param.SetUInt("key", static_cast<unsigned>(s.keyboardKey_));
            param.SetUInt("mousebutton", static_cast<unsigned>(s.mouseButton_));
            param.SetUInt("modifiers", s.modifiers_);
        }
    }
}

const Shortcut& Shortcuts::Get(const StringHash& _event) const
{
    auto it = shortcuts_.Find(_event);
    if (it != shortcuts_.End())
        return (*(*it).second_.shortcuts_.Begin());
    return Shortcut::EMPTY;
}

String Shortcuts::GetCaption(const StringHash& _event,
    const String& def /* = String::EMPTY */,
    bool widthShortcut /* = false */,
    unsigned align /* = 0 */)
{
    auto it = shortcuts_.Find(_event);
    if (it == shortcuts_.End())
        return def;
    String result = (*it).second_.name_;
    if (widthShortcut && (*it).second_.shortcuts_.Size() != 0)
    {
        const Shortcut& sc = (*it).second_.shortcuts_.Front();
        String scName = sc.ShortcutName();
        if (!scName.Empty())
        {
            if (align < result.Length())
                result += "  ";
            else
            {
                while (result.Length() < align)
                    result += " ";
            }
            result += scName;
        }
    }
    return result;
}

String Shortcuts::GetShortcutName(const StringHash& _event)
{
    auto it = shortcuts_.Find(_event);
    if (it != shortcuts_.End())
    {
        if ((*it).second_.shortcuts_.Size() != 0)
        {
            const Shortcut& sc = (*it).second_.shortcuts_.Front();
            return sc.ShortcutName();
        }
    }
    return String::EMPTY;
}

void Shortcuts::RestoreDefault()
{
    Init();
    AddDefault();
}

void Shortcuts::Delete(unsigned id)
{
    for (auto& _sc : shortcuts_)
    {
        for (auto it = _sc.second_.shortcuts_.Begin(); it != _sc.second_.shortcuts_.End(); ++it)
        {
            if ((*it).id_ == id)
            {
                _sc.second_.shortcuts_.Erase(it);
                return;
            }

        }
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

void Shortcuts::HandleUpdate(StringHash, VariantMap&)
{
    for (auto& t : triggered_)
        t.second_ = false;

    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    Input* input = GetSubsystem<Input>();
    for (const auto& sc : shortcuts_)
    {
        for (const auto& s : sc.second_.shortcuts_)
        {
            if (triggered_[sc.first_])
                continue;
            if (!IsModifier(s.keyboardKey_))
            {
                triggered_[sc.first_] = (
                    (s.keyboardKey_ != KEY_UNKNOWN ? input->GetKeyDown(s.keyboardKey_) : false) ||
                    (s.mouseButton_ != MOUSEB_NONE ? input->GetMouseButtonDown(s.mouseButton_) : false)) &&
                    (ModifiersMatch(s.modifiers_));
            }
            else
            {
                // Don't check for modifiers when the key itself is a modifier
                triggered_[sc.first_] = input->GetKeyDown(s.keyboardKey_);
            }
        }
    }
}

void Shortcuts::HandleKeyDown(StringHash, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyDown;
    bool repeat = eventData[P_REPEAT].GetBool();
    if (repeat)
        return;

    VariantMap& e = GetEventDataMap();
    Key key = static_cast<Key>(eventData[P_KEY].GetInt());
    for (const auto& sc : shortcuts_)
    {
        for (const auto& s : sc.second_.shortcuts_)
        {
            if (s.keyboardKey_ == KEY_UNKNOWN)
                continue;
            if (sc.second_.trigger_ == Trigger::Down)
            {
                if (s.keyboardKey_ != KEY_UNKNOWN && s.keyboardKey_ != key)
                    continue;
//                if (s.modifiers_ != 0 && !ModifiersMatch(s.modifiers_))
                    // If we have modifiers also these must match
                if (ModifiersMatch(s.modifiers_))
                    SendEvent(sc.second_.event_, e);
            }
        }
    }
}

void Shortcuts::HandleKeyUp(StringHash, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement())
        return;

    using namespace KeyUp;

    VariantMap& e = GetEventDataMap();
    Key key = static_cast<Key>(eventData[P_KEY].GetInt());
    for (const auto& sc : shortcuts_)
    {
        for (const auto& s : sc.second_.shortcuts_)
        {
            if (s.keyboardKey_ == KEY_UNKNOWN)
                continue;
            if (sc.second_.trigger_ == Trigger::Up)
            {
                if (s.keyboardKey_ != KEY_UNKNOWN && s.keyboardKey_ != key)
                    continue;
//                if (s.modifiers_ != 0 && !ModifiersMatch(s.modifiers_))
                    // If we have modifiers also these must match
                if (ModifiersMatch(s.modifiers_))
                    SendEvent(sc.second_.event_, e);
            }
        }
    }
}

void Shortcuts::HandleMouseDown(StringHash, VariantMap& eventData)
{
    using namespace MouseButtonDown;

    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetUInt());

    VariantMap& e = GetEventDataMap();
    for (const auto& sc : shortcuts_)
    {
        for (const auto& s : sc.second_.shortcuts_)
        {
            if (sc.second_.trigger_ == Trigger::Down)
            {
                if (s.mouseButton_ == button)
                {
//                    if (s.modifiers_ != 0 && !ModifiersMatch(s.modifiers_))
                        // If we have modifiers also these this
                    if (ModifiersMatch(s.modifiers_))
                        SendEvent(sc.second_.event_, e);
                }
            }
        }
    }
}

void Shortcuts::HandleMouseUp(StringHash, VariantMap& eventData)
{
    using namespace MouseButtonUp;

    MouseButton button = static_cast<MouseButton>(eventData[P_BUTTON].GetInt());

    VariantMap& e = GetEventDataMap();
    for (const auto& sc : shortcuts_)
    {
        for (const auto& s : sc.second_.shortcuts_)
        {
            if (sc.second_.trigger_ == Trigger::Up)
            {
                if (s.mouseButton_ == button)
                {
//                    if (s.modifiers_ != 0 && !ModifiersMatch(s.modifiers_))
                        // If we have modifiers also these this
                    if (ModifiersMatch(s.modifiers_))
                        SendEvent(sc.second_.event_, e);
                }
            }
        }
    }
}

bool Shortcuts::ModifiersMatch(unsigned mods)
{
    unsigned m = GetModifiers();
    if (m == mods)
        return true;
    if (m != 0 && mods == 0)
        return false;
    return ((m & mods) == mods);
/*    Input* input = GetSubsystem<Input>();
    return (((mods & SC_MOD_CTRL) == 0) || input->GetKeyDown(KEY_LCTRL) || input->GetKeyDown(KEY_RCTRL)) &&
        (((mods & SC_MOD_LCTRL) == 0) || input->GetKeyDown(KEY_LCTRL)) &&
        (((mods & SC_MOD_RCTRL) == 0) || input->GetKeyDown(KEY_RCTRL)) &&

        (((mods & SC_MOD_SHIFT) == 0) || input->GetKeyDown(KEY_LSHIFT) || input->GetKeyDown(KEY_RSHIFT)) &&
        (((mods & SC_MOD_LSHIFT) == 0) || input->GetKeyDown(KEY_LSHIFT)) &&
        (((mods & SC_MOD_RSHIFT) == 0) || input->GetKeyDown(KEY_RSHIFT)) &&

        (((mods & SC_MOD_ALT) == 0) || input->GetKeyDown(KEY_LALT) || input->GetKeyDown(KEY_RALT)) &&
        (((mods & SC_MOD_LALT) == 0) || input->GetKeyDown(KEY_LALT)) &&
        (((mods & SC_MOD_RALT) == 0) || input->GetKeyDown(KEY_RALT));*/
}

unsigned Shortcuts::GetModifiers() const
{
    Input* input = GetSubsystem<Input>();
    unsigned result = 0;
/*    if (input->GetKeyDown(KEY_LCTRL))
    {
        result |= SC_MOD_CTRL;
        result |= SC_MOD_LCTRL;
    }*/
    if (input->GetKeyDown(KEY_RCTRL))
    {
        result |= SC_MOD_CTRL;
        result |= SC_MOD_RCTRL;
    }
    if (input->GetKeyDown(KEY_LSHIFT))
    {
        result |= SC_MOD_SHIFT;
        result |= SC_MOD_LSHIFT;
    }
    if (input->GetKeyDown(KEY_RSHIFT))
    {
        result |= SC_MOD_SHIFT;
        result |= SC_MOD_RSHIFT;
    }
    if (input->GetKeyDown(KEY_LALT))
    {
        result |= SC_MOD_ALT;
        result |= SC_MOD_LALT;
    }
    if (input->GetKeyDown(KEY_RALT))
    {
        result |= SC_MOD_ALT;
        result |= SC_MOD_RALT;
    }
    return result;
}

bool Shortcuts::IsModifier(Key key)
{
    return (key == KEY_CTRL || key == KEY_SHIFT || key == KEY_ALT ||
        key == KEY_LCTRL || key == KEY_LSHIFT || key == KEY_LALT ||
        key == KEY_RCTRL || key == KEY_RSHIFT || key == KEY_RALT);
}

