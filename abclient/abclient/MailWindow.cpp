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

    UIElement* preview = dynamic_cast<UIElement*>(GetChild("MailPreviewPlaceholder", true));
    previewEdit_ = preview->CreateChild<MultiLineEdit>();
    previewEdit_->SetSize(preview->GetSize());
    previewEdit_->SetEditable(false);
}
