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

class HotkeyEdit : public LineEdit
{
    URHO3D_OBJECT(HotkeyEdit, LineEdit);
private:
    Key key_;
    MouseButton mouseButton_;
    int qualifiers_;
    void UpdateText();
    String GetMBName() const
    {
        switch (mouseButton_)
        {
        case MOUSEB_LEFT:
            return "LMB";
        case MOUSEB_MIDDLE:
            return "MMB";
        case MOUSEB_RIGHT:
            return "RMB";
        case MOUSEB_X1:
            return "X1MB";
        case MOUSEB_X2:
            return "X2MB";
        default:
            return String::EMPTY;
        }
    }
    String GetQualName() const
    {
        if (qualifiers_ == 0)
            return String::EMPTY;
        String result;
        if (qualifiers_ & SC_MOD_CTRL)
            result += "Ctrl+";
        if (qualifiers_ & SC_MOD_LCTRL)
            result += "LeftCtrl+";
        if (qualifiers_ & SC_MOD_RCTRL)
            result += "RightCtrl+";
        if (qualifiers_ & SC_MOD_SHIFT)
            result += "Shift+";
        if (qualifiers_ & SC_MOD_LSHIFT)
            result += "LeftShift+";
        if (qualifiers_ & SC_MOD_RSHIFT)
            result += "RightShift+";
        if (qualifiers_ & SC_MOD_ALT)
            result += "Alt+";
        if (qualifiers_ & SC_MOD_LALT)
            result += "LeftAlt+";
        if (qualifiers_ & SC_MOD_RALT)
            result += "RightAlt+";
        return result;
    }
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    HotkeyEdit(Context* context);
    ~HotkeyEdit();

    Key GetKey() const
    {
        return key_;
    }
    MouseButton GetMouseButton() const
    {
        return mouseButton_;
    }
    int GetQualifiers() const
    {
        return qualifiers_;
    }
    bool Empty() const
    {
        return key_ == KEY_UNKNOWN && mouseButton_ == MOUSEB_NONE;
    }
    void SetKey(Key key)
    {
        if (key_ != key)
        {
            key_ = key;
            UpdateText();
        }
    }
    void SetMouseButton(MouseButton button)
    {
        if (mouseButton_ != button)
        {
            mouseButton_ = button;
            UpdateText();
        }
    }
    void SetQualifiers(int qualifiers)
    {
        if (qualifiers_ != qualifiers)
        {
            qualifiers_ = qualifiers;
            UpdateText();
        }
    }
};

