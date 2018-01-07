#include "stdafx.h"
#include "MailWindow.h"

void MailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MailWindow>();
}

MailWindow::MailWindow(Context* context) :
    UIElement(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/MailWindow.xml");
    LoadChildXML(chatFile->GetRoot());

    Window* wnd = dynamic_cast<Window*>(GetChild("MailWindow", true));
    SetSize(wnd->GetSize());
    wnd->SetBringToBack(false);
    wnd->SetPriority(200);

    previewEdit_ = wnd->CreateChild<MultiLineEdit>();
    previewEdit_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    previewEdit_->SetStyle("LineEdit");
    previewEdit_->SetLayoutMode(LM_FREE);
    previewEdit_->SetAlignment(HA_LEFT, VA_TOP);
    previewEdit_->SetEditable(true);
    previewEdit_->SetMultiLine(true);
    previewEdit_->SetMaxNumLines(0);
    previewEdit_->SetMaxLength(255);
    previewEdit_->SetText("Hallo!");
    previewEdit_->ApplyAttributes();

    wnd->EnableLayoutUpdate();

    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MailWindow, HandleCloseClicked));
}

void MailWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    SetVisible(false);
}
