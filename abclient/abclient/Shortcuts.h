#pragma once

#include <SDL/SDL_keyboard.h>

#define SC_MOD_LSHIFT  1               // Left Shift
#define SC_MOD_RSHIFT  1 << 1          // Right Shift
#define SC_MOD_SHIFT   1 << 2          // Left or right Shift
#define SC_MOD_LCTRL   1 << 3          // Left Ctrl
#define SC_MOD_RCTRL   1 << 4          // Right Ctrl
#define SC_MOD_CTRL    1 << 5          // Left or right Ctrl
#define SC_MOD_LALT    1 << 6          // Left Alt
#define SC_MOD_RALT    1 << 7          // Right Alt
#define SC_MOD_ALT     1 << 8          // Left or right Alt

enum class Trigger
{
    None,
    Down,
    Up,
};

struct Shortcut
{
    Shortcut() :
        name_("Unknown"),
        trigger_(Trigger::None),
        event_(StringHash::ZERO),
        scanCode_(SCANCODE_UNKNOWN),
        keyboardKey_(KEY_UNKNOWN),
        mouseButton_(MOUSEB_NONE),
        modifiers_(0),
        customizeable_(true),
        id(0)
    { }
    Shortcut(const StringHash& _event, const String& name = "Unknown", Trigger trigger = Trigger::None,
        Scancode scanCode = SCANCODE_UNKNOWN, Key key = KEY_UNKNOWN,
        MouseButton mb = MOUSEB_NONE, unsigned mods = 0, bool customizeable = true) :
        event_(_event),
        name_(name),
        trigger_(trigger),
        scanCode_(scanCode),
        keyboardKey_(key),
        mouseButton_(mb),
        modifiers_(mods),
        customizeable_(customizeable),
        id(0)
    { }
    operator bool() const
    {
        return event_ != StringHash::ZERO;
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
    String ShortcutNameLong() const
    {
        if (keyboardKey_ != KEY_UNKNOWN)
        {
            return "[" + ModName() + String(SDL_GetKeyName(keyboardKey_)) + "]";
        }
        if (scanCode_ != SCANCODE_UNKNOWN)
            return "[" + ModName() + String(SDL_GetScancodeName(static_cast<SDL_Scancode>(scanCode_))) + "]";
        switch (mouseButton_)
        {
        case MOUSEB_LEFT:
            return "[" + ModName() + "LMB]";
        case MOUSEB_MIDDLE:
            return "[" + ModName() + "MMB]";
        case MOUSEB_RIGHT:
            return "[" + ModName() + "RMB]";
        case MOUSEB_X1:
            return "[" + ModName() + "X1MB]";
        case MOUSEB_X2:
            return "[" + ModName() + "X2MB]";
        }
        return "";
    }
    String ShortcutName() const
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
            int length = ToUnicode(static_cast<unsigned>(keyboardKey_), scanCode_, keyboardState, buff, 256, 0);
            if (length > 0)
            {
                buff[length] = '\0';
                String res(buff);
                return "[" + res + "]";
            }
        }
#endif
        return ShortcutNameLong();
    }
    String Caption(unsigned align = 0) const
    {
        String result = name_;
        String scName = ShortcutName();
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
        return result;
    }
    StringHash event_;
    String name_;
    Trigger trigger_;
    Scancode scanCode_;
    Key keyboardKey_;
    MouseButton mouseButton_;
    unsigned modifiers_;
    bool customizeable_;
    unsigned id;

    static const Shortcut EMPTY;
};

class Shortcuts : public Object
{
    URHO3D_OBJECT(Shortcuts, Object);
private:
    Vector<Shortcut> shortcuts_;
    HashMap<StringHash, bool> triggered_;
    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void Add(const Shortcut& sc);
    void Init();
    bool ModifiersMatch(unsigned mods);
public:
    Shortcuts(Context* context);
    ~Shortcuts();

    bool Test(const StringHash& e);
    void Load(const XMLElement& root);
    void Save(XMLElement& root);
    const Shortcut& Get(const StringHash& _event) const;
    bool Get(unsigned id, Shortcut& sc);
    String GetCaption(const StringHash& _event, const String& def = String::EMPTY, unsigned align = 0);
};

