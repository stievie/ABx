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

#include "stdafx.h"
#include "AddAccountKeyDialog.h"
#include "FwClient.h"
#include "LevelManager.h"
#include "BaseLevel.h"

AddAccountKeyDialog::AddAccountKeyDialog(Context* context) :
    DialogWindow(context)
{
    SetName("AddAccountKeyDialog");
    LoadLayout("UI/AddAccountKeyWindow.xml");
    SetStyleAuto();

    SetSize(320, 160);
    SetMinSize(320, 160);
    SetLayoutSpacing(10);
    UpdateLayout();
    SetMovable(false);

    MakeModal();
    BringToFront();
    accountKeyEdit_ = GetChildStaticCast<LineEdit>("AccountKeyEdit", true);
    SubscribeEvents();
}

AddAccountKeyDialog::~AddAccountKeyDialog()
{
    UnsubscribeFromAllEvents();
}

void AddAccountKeyDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    auto* addButton = GetChildStaticCast<Button>("AddButton", true);
    SubscribeToEvent(addButton, E_RELEASED, URHO3D_HANDLER(AddAccountKeyDialog, HandleAddClicked));
    auto* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(AddAccountKeyDialog, HandleCloseClicked));
}

void AddAccountKeyDialog::HandleAddClicked(StringHash, VariantMap&)
{
    auto* lm = GetSubsystem<LevelManager>();
    auto* level = lm->GetCurrentLevel<BaseLevel>();
    String accKey = accountKeyEdit_->GetText().Trimmed();
    if (accKey.Empty())
    {
        level->ShowError("Please enter or paste an account key");
        return;
    }
    if (accKey.Length() != 36)
    {
        level->ShowError("An account key has exactly 36 characters in the form of XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX");
        return;
    }

    auto* client = GetSubsystem<FwClient>();
    client->AddAccountKey(accKey);
    Close();
}

void AddAccountKeyDialog::HandleCloseClicked(StringHash, VariantMap&)
{
    Close();
}
