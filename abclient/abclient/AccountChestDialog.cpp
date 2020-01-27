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
#include "AccountChestDialog.h"
#include "FwClient.h"

AccountChestDialog::AccountChestDialog(Context* context) :
    DialogWindow(context),
    initializted_(false)
{
    SetName(AccountChestDialog::GetTypeNameStatic());

    LoadLayout("UI/AccountChestWindow.xml");
    Center();

    SetStyleAuto();

    SubscribeEvents();
}

AccountChestDialog::~AccountChestDialog()
{
    UnsubscribeFromAllEvents();
}

void AccountChestDialog::HandleChest(StringHash, VariantMap& eventData)
{
    // TODO:
    (void)eventData;
}

void AccountChestDialog::HandleChestItemUpdate(StringHash, VariantMap& eventData)
{
    // TODO:
    (void)eventData;
}

void AccountChestDialog::HandleChestItemRemove(StringHash, VariantMap& eventData)
{
    // TODO:
    (void)eventData;
}

void AccountChestDialog::HandleItemClicked(StringHash, VariantMap& eventData)
{
    // TODO:
    (void)eventData;
}

void AccountChestDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    SubscribeToEvent(Events::E_CHEST, URHO3D_HANDLER(AccountChestDialog, HandleChest));
    SubscribeToEvent(Events::E_CHESTITEMUPDATE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemUpdate));
    SubscribeToEvent(Events::E_CHESTITEMDELETE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemRemove));
}

void AccountChestDialog::Initialize()
{
    if (!initializted_)
    {
        GetSubsystem<FwClient>()->UpdateChest();
        initializted_ = true;
    }
}
