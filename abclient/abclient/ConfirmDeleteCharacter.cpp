/**
 * Copyright 2020 Stefan Ascher
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

#include "stdafx.h"
#include "ConfirmDeleteCharacter.h"

ConfirmDeleteCharacter::ConfirmDeleteCharacter(Context* context, const String& uuid, const String& name) :
    DialogWindow(context),
    uuid_(uuid),
    name_(name)
{
    SetName("ConfirmDeleteCharacter");
    LoadLayout("UI/DeleteCharacterConfirmation.xml");
    SetStyleAuto();

    SetSize(400, 230);
    SetMinSize(400, 230);
    SetMaxSize(400, 230);
    SetLayoutSpacing(10);
    SetLayoutBorder({ 10, 10, 10, 10 });

    auto* deleteButton = GetChildDynamicCast<Button>("DeleteButton", true);
    SubscribeToEvent(deleteButton, E_RELEASED, URHO3D_HANDLER(ConfirmDeleteCharacter, HandleDeleteClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(ConfirmDeleteCharacter, HandleCancelClicked));

    nameEdit_ = GetChildDynamicCast<LineEdit>("NameEdit", true);

    auto* messageText = GetChildStaticCast<Text>("MessageText", true);
    String message;
    message.AppendWithFormat("If you are sure that you want to delete %s enter %s:", name_.CString(), name_.CString());
    messageText->SetText(message);

    auto* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();
    root->AddChild(this);
    const IntVector2& size = GetSize();
    SetPosition((root->GetWidth() - size.x_) / 2, (root->GetHeight() - size.y_) / 2);

    SetModal(true);
    SubscribeToEvent(this, E_MODALCHANGED, URHO3D_HANDLER(ConfirmDeleteCharacter, HandleCancelClicked));
    nameEdit_->SetFocus(true);

    AddRef();
}

ConfirmDeleteCharacter::~ConfirmDeleteCharacter()
{
    Remove();
}

void ConfirmDeleteCharacter::HandleDeleteClicked(StringHash, VariantMap&)
{
    using namespace ConfirmDeleteChar;

    String name = nameEdit_->GetText();
    if (!name.Compare(name_))
    {
        using MsgBox = Urho3D::MessageBox;
        /* MsgBox* msgBox = */ new MsgBox(context_, "The entered name does not match the Character name.",
            "Error");
        return;
    }

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = true;
    newEventData[P_UUID] = uuid_;
    newEventData[P_NAME] = name_;
    SendEvent(E_CONFIRMDELETECHAR, newEventData);
    ReleaseRef();
}

void ConfirmDeleteCharacter::HandleCancelClicked(StringHash, VariantMap&)
{
    using namespace ConfirmDeleteChar;

    VariantMap& newEventData = GetEventDataMap();
    newEventData[P_OK] = false;
    newEventData[P_UUID] = uuid_;
    newEventData[P_NAME] = name_;
    SendEvent(E_CONFIRMDELETECHAR, newEventData);
    ReleaseRef();
}
