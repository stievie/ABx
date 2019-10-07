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
