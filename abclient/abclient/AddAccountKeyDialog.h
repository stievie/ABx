#pragma once

#include "DialogWindow.h"

class AddAccountKeyDialog : public DialogWindow
{
    URHO3D_OBJECT(AddAccountKeyDialog, DialogWindow)
private:
    void HandleAddClicked(StringHash eventType, VariantMap& eventData);
    void HandleCloseClicked(StringHash eventType, VariantMap& eventData);
protected:
    void SubscribeEvents() override;
public:
    AddAccountKeyDialog(Context* context);
    ~AddAccountKeyDialog() override;

    SharedPtr<LineEdit> accountKeyEdit_;
};
