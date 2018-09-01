#include "stdafx.h"
#include "HotkeyEdit.h"
#include <SDL/SDL_keyboard.h>

void HotkeyEdit::RegisterObject(Context* context)
{
    context->RegisterFactory<HotkeyEdit>();
}

HotkeyEdit::HotkeyEdit(Context* context) :
    LineEdit(context),
    key_(KEY_UNKNOWN),
    mouseButton_(MOUSEB_NONE),
    qualifiers_(0)
{
    SetEditable(false);
    SetCursorMovable(false);
    SetTextCopyable(false);
    SetTextSelectable(false);
    SetFocusMode(FM_FOCUSABLE);
    SubscribeToEvent(E_MOUSEBUTTONDOWN, URHO3D_HANDLER(HotkeyEdit, HandleMouseDown));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(HotkeyEdit, HandleToggleDebugHUD));
}

HotkeyEdit::~HotkeyEdit()
{
    UnsubscribeFromAllEvents();
}

void HotkeyEdit::HandleToggleDebugHUD(StringHash, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement() != this)
        return;

    using namespace KeyDown;

    if (eventData[P_REPEAT].GetBool())
        return;

    mouseButton_ = MOUSEB_NONE;
    key_ = static_cast<Key>(eventData[P_KEY].GetInt());
    qualifiers_ = 0;
    if (key_ != KEY_LSHIFT && key_ != KEY_RSHIFT && key_ != KEY_LCTRL && key_ != KEY_RCTRL && key_ != KEY_LALT && key_ != KEY_RALT)
    {
        Input* input = GetSubsystem<Input>();
        if (input->GetKeyDown(KEY_LSHIFT))
            qualifiers_ |= SC_MOD_LSHIFT;
        if (input->GetKeyDown(KEY_RSHIFT))
            qualifiers_ |= SC_MOD_RSHIFT;
        if (input->GetKeyDown(KEY_LCTRL))
            qualifiers_ |= SC_MOD_LCTRL;
        if (input->GetKeyDown(KEY_RCTRL))
            qualifiers_ |= SC_MOD_RCTRL;
        if (input->GetKeyDown(KEY_LALT))
            qualifiers_ |= SC_MOD_LALT;
        if (input->GetKeyDown(KEY_RALT))
            qualifiers_ |= SC_MOD_RALT;
    }

    UpdateText();
}

void HotkeyEdit::HandleMouseDown(StringHash, VariantMap& eventData)
{
    UI* ui = GetSubsystem<UI>();
    if (ui->GetFocusElement() != this)
        return;

    key_ = KEY_UNKNOWN;
    using namespace MouseButtonDown;
    mouseButton_ = static_cast<MouseButton>(eventData[P_BUTTON].GetInt());
    qualifiers_ = 0;
    Input* input = GetSubsystem<Input>();
    if (input->GetKeyDown(KEY_LSHIFT))
        qualifiers_ |= SC_MOD_LSHIFT;
    if (input->GetKeyDown(KEY_RSHIFT))
        qualifiers_ |= SC_MOD_RSHIFT;
    if (input->GetKeyDown(KEY_LCTRL))
        qualifiers_ |= SC_MOD_LCTRL;
    if (input->GetKeyDown(KEY_RCTRL))
        qualifiers_ |= SC_MOD_RCTRL;
    if (input->GetKeyDown(KEY_LALT))
        qualifiers_ |= SC_MOD_LALT;
    if (input->GetKeyDown(KEY_RALT))
        qualifiers_ |= SC_MOD_RALT;

    UpdateText();
}

void HotkeyEdit::UpdateText()
{
    String text;
    // Either key or mouse, never both
    if (key_ != KEY_UNKNOWN)
    {
        if (key_ != KEY_LSHIFT && key_ != KEY_RSHIFT && key_ != KEY_LCTRL && key_ != KEY_RCTRL && key_ != KEY_LALT && key_ != KEY_RALT)
            text += GetQualName();
        text += String(SDL_GetKeyName(key_));
    }
    else if (mouseButton_ != MOUSEB_NONE)
        text += GetQualName() + GetMBName();
    SetText(text);
}

