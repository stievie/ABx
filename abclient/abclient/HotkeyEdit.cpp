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
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(HotkeyEdit, HandleKeyDown));
}

HotkeyEdit::~HotkeyEdit()
{
    UnsubscribeFromAllEvents();
}

void HotkeyEdit::HandleKeyDown(StringHash, VariantMap& eventData)
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
        text += String(SDL_GetKeyName(static_cast<SDL_Keycode>(key_)));
    }
    else if (mouseButton_ != MOUSEB_NONE)
        text += GetQualName() + GetMBName();
    SetText(text);
}

