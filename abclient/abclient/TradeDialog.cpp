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
#include "TradeDialog.h"

TradeDialog::TradeDialog(Context* context, const String& title) :
    DialogWindow(context)
{
    LoadLayout("UI/TradeWindow.xml");
    SetStyleAuto();

    SetSize(368, 230);
    SetMinSize(368, 230);
    SetMaxSize(368, 230);
    SetLayoutSpacing(10);
    SetLayoutBorder({ 10, 10, 10, 10 });
    SetMovable(true);

    auto* caption = GetChildDynamicCast<Text>("Caption", true);
    caption->SetText(title);

    auto* okButton = GetChildDynamicCast<Button>("OkButton", true);
    SubscribeToEvent(okButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleOkClicked));
    auto* cancelButton = GetChildDynamicCast<Button>("CancelButton", true);
    SubscribeToEvent(cancelButton, E_RELEASED, URHO3D_HANDLER(TradeDialog, HandleCancelClicked));

    BringToFront();
}

TradeDialog::~TradeDialog()
{ }

void TradeDialog::HandleOkClicked(StringHash, VariantMap&)
{
    Close();
}

void TradeDialog::HandleCancelClicked(StringHash, VariantMap&)
{
    Close();
}
