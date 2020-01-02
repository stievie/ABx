#pragma once

#include "DialogWindow.h"

class AccountChestDialog : public DialogWindow
{
    URHO3D_OBJECT(AccountChestDialog, DialogWindow)
private:
    bool initializted_;
    void HandleChest(StringHash eventType, VariantMap& eventData);
    void HandleChestItemUpdate(StringHash eventType, VariantMap& eventData);
    void HandleChestItemRemove(StringHash eventType, VariantMap& eventData);
    void HandleItemClicked(StringHash eventType, VariantMap& eventData);
protected:
    void SubscribeEvents() override;
public:
    AccountChestDialog(Context* context);
    ~AccountChestDialog() override;
    void Initialize() override;
};

