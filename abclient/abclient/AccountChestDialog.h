#pragma once

#include "DialogWindow.h"

class AccountChestDialog : public DialogWindow
{
    URHO3D_OBJECT(AccountChestDialog, DialogWindow);
private:
    void HandleInventoryItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleInventoryItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
protected:
    void SubscribeEvents() override;
public:
    AccountChestDialog(Context* context);
    ~AccountChestDialog();
    void Initialize() override;
};

