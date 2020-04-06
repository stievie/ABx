/**
 * Copyright 2017-2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "NewMailWindow.h"
#include "Shortcuts.h"
#include "FwClient.h"
#include <AB/Entities/Limits.h>

void NewMailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<NewMailWindow>();
}

NewMailWindow::NewMailWindow(Context* context) :
    Window(context)
{
    SetName("NewMailWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/NewMailWindow.xml");
    LoadXML(file->GetRoot());

    // It seems this isn't loaded from the XML file
    SetLayoutMode(LM_VERTICAL);
    SetLayoutBorder(IntRect(4, 4, 4, 4));
    SetPivot(0, 0);
    SetOpacity(0.9f);
    SetResizable(true);
    SetMovable(true);
    Texture2D* tex = cache->GetResource<Texture2D>("Textures/UI.png");
    SetTexture(tex);
    SetImageRect(IntRect(48, 0, 64, 16));
    SetBorder(IntRect(4, 4, 4, 4));
    SetImageBorder(IntRect(0, 0, 0, 0));
    SetResizeBorder(IntRect(8, 8, 8, 8));
    SetBringToFront(true);
    SetBringToBack(true);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = GetChildStaticCast<Text>("CaptionText", true);
    caption->SetText(scs->GetCaption(Events::E_SC_TOGGLENEWMAILWINDOW, "New Mail", true));

    SetSize(500, 400);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));

    recipient_ = GetChildStaticCast<LineEdit>("RecipientEdit", true);
    subject_ = GetChildStaticCast<LineEdit>("SubjectEdit", true);

    UIElement* container = GetChild("EditorContainer", true);
    mailBody_ = container->CreateChild<MultiLineEdit>();
    mailBody_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    mailBody_->SetStyle("MultiLineEdit");
    mailBody_->SetPosition(0, 0);
    mailBody_->SetClipBorder({ 4, 4, 4, 4 });
    mailBody_->SetSize(container->GetSize());
    mailBody_->SetEditable(true);
    mailBody_->SetMultiLine(true);
    mailBody_->SetTextCopyable(true);
    mailBody_->SetTextSelectable(true);

    SetStyleAuto();

    UpdateLayout();

    SubscribeToEvents();
}

NewMailWindow::~NewMailWindow()
{
    UnsubscribeFromAllEvents();
}

void NewMailWindow::SubscribeToEvents()
{
    Button* closeButton = GetChildStaticCast<Button>("CloseButton", true);
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(NewMailWindow, HandleCloseClicked));
    Button* sendButton = GetChildStaticCast<Button>("SendMailButton", true);
    SubscribeToEvent(sendButton, E_RELEASED, URHO3D_HANDLER(NewMailWindow, HandleSendClicked));
}

void NewMailWindow::HandleCloseClicked(StringHash, VariantMap&)
{
    SetVisible(false);
}

void NewMailWindow::HandleSendClicked(StringHash, VariantMap&)
{
    const String recipient = recipient_->GetText().Trimmed();
    if (recipient.Empty())
    {
        recipient_->SetFocus(true);
        return;
    }
    String body = mailBody_->GetText().Trimmed();
    if (body.Empty())
    {
        mailBody_->SetFocus(true);
        return;
    }
    String subject = subject_->GetText();
    if (subject.Length() > AB::Entities::Limits::MAX_MAIL_SUBJECT)
        subject.Resize(AB::Entities::Limits::MAX_MAIL_SUBJECT);
    if (body.Length() > AB::Entities::Limits::MAX_MAIL_MESSAGE)
        body.Resize(AB::Entities::Limits::MAX_MAIL_MESSAGE);

    FwClient* client = GetSubsystem<FwClient>();
    client->SendMail(std::string(recipient.CString(), recipient.Length()),
        std::string(subject.CString(), subject.Length()),
        std::string(body.CString(), body.Length()));

    SetVisible(false);
    recipient_->SetText(String::EMPTY);
    subject_->SetText(String::EMPTY);
    mailBody_->SetText(String::EMPTY);
}

void NewMailWindow::SetRecipient(const String& value)
{
    recipient_->SetText(value);
}

void NewMailWindow::SetSubject(const String& value)
{
    subject_->SetText(value);
}

const String& NewMailWindow::GetSubject() const
{
    return subject_->GetText();
}

void NewMailWindow::FocusBody()
{
    mailBody_->SetFocus(true);
}

void NewMailWindow::SetBody(const String& value)
{
    mailBody_->SetText(value);
}

void NewMailWindow::FocusRecipient()
{
    recipient_->GetTextElement()->SetSelection(0);
    recipient_->SetFocus(true);
}
