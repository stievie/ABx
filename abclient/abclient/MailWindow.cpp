#include "stdafx.h"
#include "MailWindow.h"

void MailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MailWindow>();
}

MailWindow::MailWindow(Context* context) :
    Object(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/MailWindow.xml");
    if (!file)
        return;

    UI* ui = GetSubsystem<UI>();
    UIElement* root = ui->GetRoot();
    {
        SharedPtr<UIElement> holder = ui->LoadLayout(file, root->GetDefaultStyle());
        if (!holder)    // Error is already logged
            return;
        window_ = holder;
        root->AddChild(window_);    // Take ownership of the object before SharedPtr goes out of scope
    }
//    window_->SetStyleAuto();

    previewEdit_ = window_->CreateChild<MultiLineEdit>();
    previewEdit_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    previewEdit_->SetStyle("MultiLineEdit");
    previewEdit_->SetLayoutMode(LM_FREE);
    previewEdit_->SetAlignment(HA_LEFT, VA_TOP);
    previewEdit_->SetEditable(true);
    previewEdit_->SetMultiLine(true);
    previewEdit_->SetMaxNumLines(0);
    previewEdit_->SetClipBorder(IntRect(4, 4, 4, 4));
    previewEdit_->SetMaxLength(255);
    previewEdit_->SetText("is simply dummy text of the printing and typesetting industry.\n"
        "been the industry's standard dummy text ever since the 1500s\n");
    previewEdit_->ApplyAttributes();

    Button* closeButton = dynamic_cast<Button*>(window_->GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MailWindow, HandleCloseClicked));

    window_->EnableLayoutUpdate();

    // Increase reference count to keep Self alive
    AddRef();
}

void MailWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    // Self destruct
    ReleaseRef();
}
