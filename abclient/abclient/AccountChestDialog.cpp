#include "stdafx.h"
#include "AccountChestDialog.h"
#include "AbEvents.h"
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
}

void AccountChestDialog::HandleChestItemUpdate(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::HandleChestItemRemove(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::HandleItemClicked(StringHash, VariantMap& eventData)
{
}

void AccountChestDialog::SubscribeEvents()
{
    DialogWindow::SubscribeEvents();
    SubscribeToEvent(AbEvents::E_CHEST, URHO3D_HANDLER(AccountChestDialog, HandleChest));
    SubscribeToEvent(AbEvents::E_CHESTITEMUPDATE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemUpdate));
    SubscribeToEvent(AbEvents::E_CHESTITEMDELETE, URHO3D_HANDLER(AccountChestDialog, HandleChestItemRemove));
}

void AccountChestDialog::Initialize()
{
    if (!initializted_)
    {
        GetSubsystem<FwClient>()->UpdateChest();
        initializted_ = true;
    }
}
