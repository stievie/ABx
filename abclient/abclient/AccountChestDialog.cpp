#include "stdafx.h"
#include "AccountChestDialog.h"
#include "AbEvents.h"

void AccountChestDialog::HandleInventoryItemUpdate(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::HandleInventoryItemRemove(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::HandleItemClicked(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    SubscribeToEvent(AbEvents::E_INVENTORYITEMUPDATE, URHO3D_HANDLER(AccountChestDialog, HandleInventoryItemUpdate));
    SubscribeToEvent(AbEvents::E_INVENTORYITEMDELETE, URHO3D_HANDLER(AccountChestDialog, HandleInventoryItemRemove));
}

AccountChestDialog::AccountChestDialog(Context* context) :
    DialogWindow(context)
{
    SetName(AccountChestDialog::GetTypeNameStatic());

    LoadLayout("UI/AccountChestWindow.xml");
    Center();

    SetStyleAuto();

    SubscribeEvents();
}

AccountChestDialog::~AccountChestDialog()
{
}

void AccountChestDialog::Initialize()
{
}
