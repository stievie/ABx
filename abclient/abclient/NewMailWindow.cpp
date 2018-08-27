#include "stdafx.h"
#include "NewMailWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"

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

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("CaptionText", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLENEWMAILWINDOW, "New Mail"));

    SetSize(500, 400);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));

    recipient_ = dynamic_cast<LineEdit*>(GetChild("RecipientEdit", true));
    subject_ = dynamic_cast<LineEdit*>(GetChild("SubjectEdit", true));

    UIElement* container = dynamic_cast<UIElement*>(GetChild("EditorContainer", true));
    mailBody_ = container->CreateChild<MultiLineEdit>();
    mailBody_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    mailBody_->SetStyle("MultiLineEdit");
    mailBody_->SetPosition(0, 0);
    mailBody_->SetSize(container->GetSize());
    mailBody_->SetEditable(true);
    mailBody_->SetMultiLine(true);
    mailBody_->SetTextCopyable(true);
    mailBody_->SetTextSelectable(true);

    SetStyleAuto();

    UpdateLayout();

    SubscribeToEvents();
}

void NewMailWindow::SubscribeToEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(NewMailWindow, HandleCloseClicked));
    Button* sendButton = dynamic_cast<Button*>(GetChild("SendMailButton", true));
    SubscribeToEvent(sendButton, E_RELEASED, URHO3D_HANDLER(NewMailWindow, HandleSendClicked));
}

void NewMailWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    SetVisible(false);
}

void NewMailWindow::HandleSendClicked(StringHash eventType, VariantMap& eventData)
{
    if (recipient_->GetText().Trimmed().Empty())
    {
        recipient_->SetFocus(true);
        return;
    }
    if (mailBody_->GetText().Trimmed().Empty())
    {
        mailBody_->SetFocus(true);
        return;
    }

    AB::GameProtocol::CommandTypes type = AB::GameProtocol::CommandTypeMailSend;
    // /mail <name>, [<subject>:] <message>
    std::stringstream ss;
    ss << std::string(recipient_->GetText().Trimmed().CString());
    ss << ", ";
    if (!subject_->GetText().Trimmed().Empty())
    {
        ss << std::string(subject_->GetText().Trimmed().CString());
        ss << ": ";
    }
    ss << std::string(mailBody_->GetText().CString(), mailBody_->GetText().Length());
    FwClient* client = context_->GetSubsystem<FwClient>();
    std::string cmd = ss.str();
    client->Command(type, String(cmd.c_str(), (unsigned)cmd.length()).Substring(0, MAX_CHAT_MESSAGE));
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
