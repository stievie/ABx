#pragma once

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
        modifiers_(0)
    { }
    Shortcut(const String& name, Trigger trigger, const StringHash& _event,
        Scancode scanCode = SCANCODE_UNKNOWN, Key key = KEY_UNKNOWN,
        MouseButton mb = MOUSEB_NONE, unsigned mods = 0) :
        name_(name),
        trigger_(trigger),
        event_(_event),
        scanCode_(scanCode),
        keyboardKey_(key),
        mouseButton_(mb),
        modifiers_(mods)
    { }
    String name_;
    Trigger trigger_;
    StringHash event_;
    Scancode scanCode_;
    Key keyboardKey_;
    MouseButton mouseButton_;
    unsigned modifiers_;
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
    void AddDefault();
    bool ModifiersMatch(unsigned mods);
public:
    Shortcuts(Context* context);
    ~Shortcuts();

    bool Test(const StringHash& e);
    void Add(const Shortcut& sc);
    void Load(const XMLElement& root);
    void Save(XMLElement& root);
};

