#include "stdafx.h"
#include "MailWindow.h"
#include "Shortcuts.h"
#include "AbEvents.h"
#include "FwClient.h"
#include <Mustache/mustache.hpp>
#include <TimeUtils.h>

void MailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MailWindow>();
}

MailWindow::MailWindow(Context* context) :
    Window(context)
{
    SetName("MailWindow");

    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* file = cache->GetResource<XMLFile>("UI/MailWindow.xml");
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

    mailList_ = dynamic_cast<ListView*>(GetChild("MailList", true));

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    Text* caption = dynamic_cast<Text*>(GetChild("CaptionText", true));
    caption->SetText(scs->GetCaption(AbEvents::E_SC_TOGGLEMAILWINDOW, "Mail"));

    SetSize(500, 400);
    auto* graphics = GetSubsystem<Graphics>();
    SetPosition(graphics->GetWidth() - GetWidth() - 5, graphics->GetHeight() / 2 - (GetHeight() / 2));

    UIElement* container = dynamic_cast<UIElement*>(GetChild("EditorContainer", true));
    mailBody_ = container->CreateChild<MultiLineEdit>();
    mailBody_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    mailBody_->SetStyle("MultiLineEdit");
    mailBody_->SetEditable(false);
    mailBody_->SetMultiLine(true);
    mailBody_->SetTextCopyable(true);
    mailBody_->SetTextSelectable(true);
    mailBody_->SetPosition(0, 0);
    mailBody_->SetSize(container->GetSize());

    SetStyleAuto();

    UpdateLayout();

    SubscribeToEvents();

    FwClient* net = context_->GetSubsystem<FwClient>();
    net->GetMailHeaders();
}

void MailWindow::SubscribeToEvents()
{
    Button* closeButton = dynamic_cast<Button*>(GetChild("CloseButton", true));
    SubscribeToEvent(closeButton, E_RELEASED, URHO3D_HANDLER(MailWindow, HandleCloseClicked));
    SubscribeToEvent(AbEvents::E_MAILINBOX, URHO3D_HANDLER(MailWindow, HandleMailInboxMessage));
    SubscribeToEvent(AbEvents::E_MAILREAD, URHO3D_HANDLER(MailWindow, HandleMailReadMessage));
    Button* newButton = dynamic_cast<Button*>(GetChild("NewMailButton", true));
    SubscribeToEvent(newButton, E_RELEASED, URHO3D_HANDLER(MailWindow, HandleNewClicked));
    Button* deleteButton = dynamic_cast<Button*>(GetChild("DeleteMailButton", true));
    SubscribeToEvent(deleteButton, E_RELEASED, URHO3D_HANDLER(MailWindow, HandleDeleteClicked));
    SubscribeToEvent(mailList_, E_ITEMSELECTED, URHO3D_HANDLER(MailWindow, HandleItemSelected));
    SubscribeToEvent(mailList_, E_ITEMDESELECTED, URHO3D_HANDLER(MailWindow, HandleItemSelected));
    SubscribeToEvent(AbEvents::E_NEWMAIL, URHO3D_HANDLER(MailWindow, HandleNewMail));
}

void MailWindow::AddItem(const String& text, const String& style, const AB::Entities::MailHeader& header)
{
    Text* txt = mailList_->CreateChild<Text>();

    txt->SetText(text);
    txt->SetVar("uuid", String(header.uuid.c_str()));
    txt->SetStyle(style);
    txt->EnableLayoutUpdate();
    txt->SetWordwrap(false);
    txt->UpdateLayout();
    mailList_->AddItem(txt);
    mailList_->EnableLayoutUpdate();
    mailList_->UpdateLayout();
}

void MailWindow::HandleCloseClicked(StringHash eventType, VariantMap& eventData)
{
    SetVisible(false);
}

void MailWindow::HandleMailInboxMessage(StringHash eventType, VariantMap& eventData)
{
    mailList_->RemoveAllItems();

    FwClient* client = context_->GetSubsystem<FwClient>();
    const std::vector<AB::Entities::MailHeader>& headers = client->GetCurrentMailHeaders();
    kainjow::mustache::mustache tpl{ "#{{index}}: <{{from}}> on {{date}}: {{subject}}" };
    unsigned i = 0;
    for (const auto& header : headers)
    {
        ++i;
        kainjow::mustache::data data;
        data.set("index", std::to_string(i));
        data.set("from", header.fromName);
        data.set("subject", (header.subject.empty() ? "(No Subject)" : header.subject));
        data.set("date", Client::format_tick(header.created));
        std::string t = tpl.render(data);
        AddItem(String(t.c_str(), (unsigned)t.size()),
            header.isRead ? "MailListItem" : "MailListItemUnread", header);
    }
}

void MailWindow::HandleMailReadMessage(StringHash eventType, VariantMap& eventData)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    const AB::Entities::Mail mail = client->GetCurrentMail();
    mailBody_->SetText(String(mail.message.c_str(), (unsigned)mail.message.size()));
}

void MailWindow::HandleNewClicked(StringHash eventType, VariantMap& eventData)
{
    VariantMap& e = GetEventDataMap();
    SendEvent(AbEvents::E_SC_TOGGLENEWMAILWINDOW, e);
}

void MailWindow::HandleDeleteClicked(StringHash eventType, VariantMap& eventData)
{
    using namespace ItemSelected;
    int selIndex = eventData[P_SELECTION].GetInt();
    Text* sel = dynamic_cast<Text*>(mailList_->GetItem(selIndex));
    if (sel)
    {
        String uuid = sel->GetVar("uuid").GetString();
        FwClient* net = context_->GetSubsystem<FwClient>();
        net->DeleteMail(std::string(uuid.CString()));
        net->GetMailHeaders();
        mailBody_->SetText(String::EMPTY);
    }
}

void MailWindow::HandleItemSelected(StringHash eventType, VariantMap& eventData)
{
    Text* sel = dynamic_cast<Text*>(mailList_->GetSelectedItem());
    if (sel)
    {
        String uuid = sel->GetVar("uuid").GetString();
        FwClient* net = context_->GetSubsystem<FwClient>();
        net->ReadMail(std::string(uuid.CString()));
        sel->SetStyle("MailListItem");
    }
}

void MailWindow::HandleItemUnselected(StringHash eventType, VariantMap& eventData)
{
    mailBody_->SetText(String::EMPTY);
}

void MailWindow::HandleNewMail(StringHash eventType, VariantMap& eventData)
{
    FwClient* net = context_->GetSubsystem<FwClient>();
    net->GetMailHeaders();
}
