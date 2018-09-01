#pragma once

#include <SDL/SDL_keyboard.h>
#include "HotkeyEdit.h"

enum class Trigger
{
    None,
    Down,
    Up,
};

struct Shortcut
{
    Shortcut(Key key = KEY_UNKNOWN,
        MouseButton mb = MOUSEB_NONE, unsigned mods = 0) :
        keyboardKey_(key),
        mouseButton_(mb),
        modifiers_(mods),
        id_(0)
    { }
    operator bool() const
    {
        return keyboardKey_ != KEY_UNKNOWN && mouseButton_ != MOUSEB_NONE;
    }
    bool operator ==(const Shortcut& rhs) const
    {
        return keyboardKey_ == rhs.keyboardKey_ && mouseButton_ == rhs.modifiers_ && modifiers_ == rhs.modifiers_;
    }
    bool operator !=(const Shortcut& rhs) const
    {
        return keyboardKey_ != rhs.keyboardKey_ || mouseButton_ != rhs.modifiers_ || modifiers_ != rhs.modifiers_;
    }
    String ModName() const
    {
        if (modifiers_ == 0)
            return "";
        String result;
        if (modifiers_ & SC_MOD_CTRL)
            result += "Ctrl+";
        if (modifiers_ & SC_MOD_LCTRL)
            result += "LeftCtrl+";
        if (modifiers_ & SC_MOD_RCTRL)
            result += "RightCtrl+";
        if (modifiers_ & SC_MOD_SHIFT)
            result += "Shift+";
        if (modifiers_ & SC_MOD_LSHIFT)
            result += "LeftShift+";
        if (modifiers_ & SC_MOD_RSHIFT)
            result += "RightShift+";
        if (modifiers_ & SC_MOD_ALT)
            result += "Alt+";
        if (modifiers_ & SC_MOD_LALT)
            result += "LeftAlt+";
        if (modifiers_ & SC_MOD_RALT)
            result += "RightAlt+";
        return result;
    }
    String ShortcutNameLong(bool plain = false) const
    {
        if (keyboardKey_ != KEY_UNKNOWN)
        {
            String result = ModName() + String(SDL_GetKeyName(keyboardKey_));
            if (plain)
                return result;
            return "[" + result + "]";
        }
        String result;
        switch (mouseButton_)
        {
        case MOUSEB_LEFT:
            result = ModName() + "LMB";
            break;
        case MOUSEB_MIDDLE:
            result = ModName() + "MMB";
            break;
        case MOUSEB_RIGHT:
            result = ModName() + "RMB";
            break;
        case MOUSEB_X1:
            result = ModName() + "X1MB";
            break;
        case MOUSEB_X2:
            result = ModName() + "X2MB";
        }
        if (!result.Empty())
        {
            if (plain)
                return result;
            return "[" + result + "]";
        }
        return String::EMPTY;
    }
    String ShortcutName(bool plain = false) const
    {
#ifdef _WIN32
        if (modifiers_ != 0)
        {
            unsigned char keyboardState[256] = {};
            if (modifiers_ & SC_MOD_CTRL)
                keyboardState[VK_CONTROL] = 0xff;
            if (modifiers_ & SC_MOD_LCTRL)
                keyboardState[VK_LCONTROL] = 0xff;
            if (modifiers_ & SC_MOD_RCTRL)
                keyboardState[VK_RCONTROL] = 0xff;
            if (modifiers_ & SC_MOD_SHIFT)
                keyboardState[VK_SHIFT] = 0xff;
            if (modifiers_ & SC_MOD_LSHIFT)
                keyboardState[VK_LSHIFT] = 0xff;
            if (modifiers_ & SC_MOD_RSHIFT)
                keyboardState[VK_RSHIFT] = 0xff;
            if (modifiers_ & SC_MOD_ALT)
                keyboardState[VK_MENU] = 0xff;
            if (modifiers_ & SC_MOD_LALT)
                keyboardState[VK_LMENU] = 0xff;
            if (modifiers_ & SC_MOD_RALT)
                keyboardState[VK_RMENU] = 0xff;
            wchar_t buff[256];
            int length = ToUnicode(static_cast<unsigned>(keyboardKey_), 0, keyboardState, buff, 256, 0);
            if (length > 0)
            {
                buff[length] = '\0';
                String res(buff);
                if (plain)
                    return res;
                return "[" + res + "]";
            }
        }
#endif
        return ShortcutNameLong(plain);
    }

    Key keyboardKey_;
    MouseButton mouseButton_;
    unsigned modifiers_;
    unsigned id_;
    static const Shortcut EMPTY;
};

struct ShortcutEvent
{
    ShortcutEvent() :
        event_(StringHash::ZERO),
        name_("Unknown"),
        trigger_(Trigger::None),
        description_(String::EMPTY),
        customizeable_(false)
    {}
    ShortcutEvent(const StringHash& _event, const String& name, Trigger trigger,
        const String& description = String::EMPTY, bool customizeable = true) :
        event_(_event),
        name_(name),
        trigger_(trigger),
        description_(description),
        customizeable_(customizeable)
    {
    }

    const String& GetDescription() const
    {
        if (!description_.Empty())
            return description_;
        return name_;
    }

    StringHash event_;
    String name_;
    Trigger trigger_;
    String description_;
    bool customizeable_;
    Vector<Shortcut> shortcuts_;
};

class Shortcuts : public Object
{
    URHO3D_OBJECT(Shortcuts, Object);
private:
    HashMap<StringHash, bool> triggered_;
    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleToggleDebugHUD(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void Init();
    void AddDefault();
    bool ModifiersMatch(unsigned mods);
    static unsigned shortcutIds;
public:
    Shortcuts(Context* context);
    ~Shortcuts();

    bool Test(const StringHash& e);
    void Load(const XMLElement& root);
    void Save(XMLElement& root);
    /// Return ID
    unsigned Add(const StringHash& _event, const Shortcut& sc);
    const Shortcut& Get(const StringHash& _event) const;
    String GetCaption(const StringHash& _event, const String& def = String::EMPTY,
        bool widthShortcut = false, unsigned align = 0);
    String GetShortcutName(const StringHash& _event);
    void RestoreDefault();
    void Delete(unsigned id);

    HashMap<StringHash, ShortcutEvent> shortcuts_;
};

