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

    accountKeyEdit_ = dynamic_cast<LineEdit*>(GetChild("AccountKeyEdit", true));
    SubscribeEvents();
}

AddAccountKeyDialog::~AddAccountKeyDialog()
{
    UnsubscribeFromAllEvents();
}

void AddAccountKeyDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    auto addButton = dynamic_cast<Button*>(GetChild("AddButton", true));
    SubscribeToEvent(addButton, E_RELEASED, URHO3D_HANDLER(AddAccountKeyDialog, HandleAddClicked));
    auto closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
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
}

void AddAccountKeyDialog::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}
