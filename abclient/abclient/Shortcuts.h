/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

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
    String ModName() const;
    String ShortcutNameLong(bool plain = false) const;
    String ShortcutName(bool plain = false) const;

    Key keyboardKey_;
    MouseButton mouseButton_;
    unsigned modifiers_;
    /// Temporary ID needed for customization
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
    URHO3D_OBJECT(Shortcuts, Object)
private:
    HashMap<StringHash, bool> triggered_;
    void SubscribeToEvents();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleKeyUp(StringHash eventType, VariantMap& eventData);
    void HandleMouseDown(StringHash eventType, VariantMap& eventData);
    void HandleMouseUp(StringHash eventType, VariantMap& eventData);
    void Init();
    void AddDefault();
    bool ModifiersMatch(unsigned mods);
    unsigned GetModifiers() const;
    static unsigned shortcutIds;
    static bool IsModifier(Key key);
public:
    Shortcuts(Context* context);
    ~Shortcuts() override;

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

