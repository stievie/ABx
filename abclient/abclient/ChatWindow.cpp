#include "stdafx.h"
#include "ChatWindow.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "Utils.h"
#include <TimeUtils.h>
#include <Mustache/mustache.hpp>

#include <Urho3D/DebugNew.h>

ChatWindow::ChatWindow(Context* context) :
    UIElement(context),
    tabIndexWhisper_(-1)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/ChatWindow.xml");
    LoadChildXML(chatFile->GetRoot());

    // Set self to same size as the window so align works
    Window* wnd = dynamic_cast<Window*>(GetChild("ChatWindow", true));
    SetSize(wnd->GetSize());
    wnd->SetBringToBack(false);
    wnd->SetPriority(200);

    tabgroup_ = wnd->CreateChild<TabGroup>();
    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_BOTTOM);
    tabgroup_->SetSize(500, 40);
    tabgroup_->SetColor(Color(0, 0, 0, 0));
    tabgroup_->SetStyleAuto();
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannelGeneral);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannelGuild);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannelParty);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannelTrade);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannelWhisper);

    tabgroup_->SetEnabled(true);
    SubscribeToEvent(E_TABSELECTED, URHO3D_HANDLER(ChatWindow, HandleTabSelected));

    chatLog_ = dynamic_cast<ListView*>(GetChild("ChatLog", true));

    SubscribeToEvent(AbEvents::E_SERVERMESSAGE, URHO3D_HANDLER(ChatWindow, HandleServerMessage));
    SubscribeToEvent(AbEvents::E_CHATMESSAGE, URHO3D_HANDLER(ChatWindow, HandleChatMessage));
    SubscribeToEvent(AbEvents::E_MAILINBOX, URHO3D_HANDLER(ChatWindow, HandleMailInboxMessage));
    SubscribeToEvent(AbEvents::E_MAILREAD, URHO3D_HANDLER(ChatWindow, HandleMailReadMessage));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ChatWindow, HandleKeyDown));

    static bool firstStart = true;
    if (firstStart)
    {
        AddLine("Hi, type /help for available commands.", "ChatLogServerInfoText");
        firstStart = false;
    }
}

void ChatWindow::FocusEdit()
{
    int sel = tabgroup_->GetSelectedIndex();
    TabElement* elem = tabgroup_->GetTabElement(sel);
    LineEdit* edit = GetActiveLineEdit();
    if (edit && !edit->HasFocus())
        edit->SetFocus(true);
}

void ChatWindow::CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatMessageChannel channel)
{
    static const IntVector2 tabSize(60, 20);
    static const IntVector2 tabBodySize(500, 20);

    ResourceCache* cache = GetSubsystem<ResourceCache>();
    TabElement *tabElement = tabs->CreateTab(tabSize, tabBodySize);
    tabElement->tabText_->SetFont(cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf"), 10);
    switch (channel)
    {
    case AB::GameProtocol::ChatChannelGeneral:
        tabElement->tabText_->SetText("General");
        break;
    case AB::GameProtocol::ChatChannelParty:
        tabElement->tabText_->SetText("Party");
        break;
    case AB::GameProtocol::ChatChannelGuild:
        tabElement->tabText_->SetText("Guild");
        break;
    case AB::GameProtocol::ChatChannelTrade:
        tabElement->tabText_->SetText("Trade");
        break;
    case AB::GameProtocol::ChatChannelWhisper:
        tabElement->tabText_->SetText("Whisper");
        tabIndexWhisper_ = tabs->GetTabCount() - 1;
        break;
    }

    UIElement* parent = tabElement->tabBody_;
    if (channel == AB::GameProtocol::ChatChannelWhisper)
    {
        UIElement* container = parent->CreateChild<UIElement>();
        container->SetSize(parent->GetSize());
        //        container->SetLayoutMode(LM_HORIZONTAL);
        LineEdit* nameEdit = container->CreateChild<LineEdit>();
        nameEdit->SetPosition(0, 0);
        nameEdit->SetName("WhisperChatNameEdit");
        nameEdit->SetAlignment(HA_LEFT, VA_CENTER);
        nameEdit->SetStyle("ChatLineEdit");
        nameEdit->SetSize(150, 20);
        parent = container;
    }

    LineEdit* edit = parent->CreateChild<LineEdit>();
    edit->SetName("ChatLineEdit");
    edit->SetVar("Channel", static_cast<int>(channel));
    if (channel != AB::GameProtocol::ChatChannelWhisper)
    {
        edit->SetPosition(0, 0);
        edit->SetSize(tabElement->tabBody_->GetSize());
        edit->SetAlignment(HA_CENTER, VA_CENTER);
    }
    else
    {
        edit->SetPosition(150, 0);
        edit->SetSize(tabElement->tabBody_->GetSize().x_ - 100, tabElement->tabBody_->GetSize().y_);
        edit->SetAlignment(HA_LEFT, VA_CENTER);
    }
    edit->SetStyle("ChatLineEdit");
    edit->SetHeight(20);
    SubscribeToEvent(edit, E_TEXTFINISHED, URHO3D_HANDLER(ChatWindow, HandleTextFinished));
    SubscribeToEvent(edit, E_FOCUSED, URHO3D_HANDLER(ChatWindow, HandleEditFocused));
    SubscribeToEvent(edit, E_DEFOCUSED, URHO3D_HANDLER(ChatWindow, HandleEditDefocused));

    tabElement->tabText_->SetColor(Color(1.0f, 1.0f, 1.0f));
}

LineEdit* ChatWindow::GetActiveLineEdit()
{
    return GetLineEdit(tabgroup_->GetSelectedIndex());
}

LineEdit* ChatWindow::GetLineEdit(int index)
{
    TabElement* elem = tabgroup_->GetTabElement(index);
    if (!elem)
        return nullptr;
    LineEdit* edit = dynamic_cast<LineEdit*>(elem->tabBody_->GetChild("ChatLineEdit", true));
    return edit;
}

void ChatWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<ChatWindow>();
}

void ChatWindow::HandleServerMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(eventData[P_MESSAGETYPE].GetInt());
    switch (type)
    {
    case AB::GameProtocol::ServerMessageTypeInfo:
        HandleServerMessageInfo(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeRoll:
        HandleServerMessageRoll(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeAge:
        HandleServerMessageAge(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypePlayerNotOnline:
        HandleServerMessagePlayerNotOnline(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypePlayerGotMessage:
        HandleServerMessagePlayerGotMessage(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeNewMail:
        HandleServerMessageNewMail(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeMailSent:
        HandleServerMessageMailSent(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeMailNotSent:
        HandleServerMessageMailNotSent(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeMailboxFull:
        HandleServerMessageMailboxFull(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeMailDeleted:
        HandleServerMessageMailDeleted(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeServerId:
        HandleServerMessageServerId(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeUnknownCommand:
        HandleServerMessageUnknownCommand(eventData);
        break;
    }
}

void ChatWindow::HandleServerMessageUnknownCommand(VariantMap&)
{
    AddLine("Unknown command", "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageInfo(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String message = eventData[P_DATA].GetString();
    AddLine(message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageRoll(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String message = eventData[P_DATA].GetString();
    String sender = eventData[P_SENDER].GetString();
    unsigned p = message.Find(":");
    String res = message.Substring(0, p);
    String max = message.Substring(p + 1);
    kainjow::mustache::mustache tpl{ "{{name}} rolls {{res}} on a {{max}} sided die." };
    kainjow::mustache::data data;
    data.set("name", std::string(sender.CString(), sender.Length()));
    data.set("res", std::string(res.CString(), res.Length()));
    data.set("max", std::string(max.CString(), max.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogChatText");
}

void ChatWindow::HandleServerMessageAge(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String message = eventData[P_DATA].GetString();
    unsigned p = message.Find(":");
    String age = message.Substring(0, p);
    String playTime = message.Substring(p + 1);
    // Seconds
    uint32_t uAge = std::atoi(age.CString());
    // Seconds
    uint32_t uPlayTime = std::atoi(playTime.CString());
    Client::TimeSpan tAge(uAge);
    std::stringstream ss;
    ss << "You have played this characters for ";
    uint32_t hours = uPlayTime / 3600;
    if (hours > 0)
        uPlayTime -= hours * 3600;
    uint32_t minutes = uPlayTime / 60;
    if (minutes > 0)
        uPlayTime -= minutes * 60;
    if (hours > 0)
        ss << hours << " hour(s) ";
    ss << minutes << " minute(s) over the past ";
    if (tAge.months > 0)
        ss << tAge.months << " month(s).";
    else
        ss << tAge.days << " day(s).";
    AddLine(String(ss.str().c_str()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerNotOnline(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String data = eventData[P_DATA].GetString();
    AddLine("Player " + data + " is not online.", "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerGotMessage(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String name = eventData[P_SENDER].GetString();
    String data = eventData[P_DATA].GetString();
    AddLine("{" + name + "} " + data, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageNewMail(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String count = eventData[P_DATA].GetString();
    kainjow::mustache::mustache tpl{ "You got a new mail, total {{count}} mail(s)." };
    kainjow::mustache::data data;
    data.set("count", std::string(count.CString(), count.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailSent(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String name = eventData[P_SENDER].GetString();
    kainjow::mustache::mustache tpl{ "Mail to {{recipient}} was sent." };
    kainjow::mustache::data data;
    data.set("recipient", std::string(name.CString(), name.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailNotSent(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String name = eventData[P_SENDER].GetString();
    kainjow::mustache::mustache tpl{ "Mail to {{recipient}} was not sent. Please check the name, or the mail box is full." };
    kainjow::mustache::data data;
    data.set("recipient", std::string(name.CString(), name.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailboxFull(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String count = eventData[P_DATA].GetString();
    kainjow::mustache::mustache tpl{ "Your mailbox is full! You have {{count}} mails. Please delete some, so people are able to send you mails." };
    kainjow::mustache::data data;
    data.set("count", std::string(count.CString(), count.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailDeleted(VariantMap& eventData)
{
    AddLine("The mail was deleted.", "ChatLogServerInfoText");
}

void ChatWindow::HandleChatMessage(StringHash eventType, VariantMap& eventData)
{
    using namespace AbEvents::ChatMessage;
    AB::GameProtocol::ChatMessageChannel channel =
        static_cast<AB::GameProtocol::ChatMessageChannel>(eventData[P_MESSAGETYPE].GetInt());
    String message = eventData[P_DATA].GetString();
    String sender = eventData[P_SENDER].GetString();
    uint32_t senderId = static_cast<uint32_t>(eventData[P_SENDERID].GetInt());
    AddChatLine(senderId, sender, message, channel);
}

void ChatWindow::HandleTabSelected(StringHash eventType, VariantMap& eventData)
{
    using namespace TabSelected;
    int idx = eventData[P_INDEX].GetInt();
    int oldIdx = eventData[P_OLD_INDEX].GetInt();
    LineEdit* edit = GetLineEdit(idx);
    if (edit)
        edit->SetFocus(true);
}

void ChatWindow::HandleKeyDown(StringHash eventType, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();
    if (key == KEY_RETURN)
        FocusEdit();
}

void ChatWindow::HandleNameClicked(StringHash eventType, VariantMap& eventData)
{
    using namespace Click;
    Text* text = dynamic_cast<Text*>(eventData[P_ELEMENT].GetPtr());
    const String& name = text->GetVar("Name").GetString();
    if (!name.Empty())
    {
        LineEdit* nameEdit = dynamic_cast<LineEdit*>(GetChild("WhisperChatNameEdit", true));
        nameEdit->SetText(name);
        tabgroup_->SetSelectedIndex(tabIndexWhisper_);
    }
}

void ChatWindow::HandleMailInboxMessage(StringHash eventType, VariantMap& eventData)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    const std::vector<AB::Entities::MailHeader>& headers = client->GetCurrentMailHeaders();
    {
        kainjow::mustache::mustache tpl{ "You have {{count}} mails:" };
        kainjow::mustache::data data;
        data.set("count", std::to_string(headers.size()));
        std::string t = tpl.render(data);
        AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogChatText");
    }
    kainjow::mustache::mustache tpl{ "#{{index}}: <{{from}}> on {{date}}: {{subject}}" };
    unsigned i = 0;
    for (const auto& header : headers)
    {
        ++i;
        kainjow::mustache::data data;
        data.set("index", std::to_string(i));
        data.set("from", header.fromName);
        data.set("subject", header.subject);
        data.set("date", Client::format_tick(header.created));
        std::string t = tpl.render(data);
        AddLine(String(header.fromName.c_str()),
            String(t.c_str(), (unsigned)t.size()),
            header.isRead ? "ChatLogMailText" : "ChatLogMailUnreadText");
    }
}

void ChatWindow::HandleMailReadMessage(StringHash eventType, VariantMap& eventData)
{
    FwClient* client = context_->GetSubsystem<FwClient>();
    const AB::Entities::Mail mail = client->GetCurrentMail();
    {
        kainjow::mustache::mustache tpl{ "{{from}} to {{to}} on {{date}}" };
        kainjow::mustache::data data;
        data.set("from", mail.fromName);
        data.set("to", mail.toName);
        data.set("date", Client::format_tick(mail.created));
        std::string t = tpl.render(data);
        AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogMailText");
    }
    AddLine(String(mail.subject.c_str(), (unsigned)mail.subject.size()), "ChatLogMailText");
    AddLine(String(mail.message.c_str(), (unsigned)mail.message.size()), "ChatLogMailText");
}

void ChatWindow::HandleServerMessageServerId(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    String id = eventData[P_DATA].GetString();
    AddLine(id, "ChatLogServerInfoText");
}

void ChatWindow::ParseChatCommand(const String& text, AB::GameProtocol::ChatMessageChannel defChannel)
{
    AB::GameProtocol::CommandTypes type = AB::GameProtocol::CommandTypeUnknown;
    String data;
    if (text.StartsWith("/"))
    {
        String cmd;
        unsigned pos = 1;
        while (pos < text.Length())
        {
            char ch = text.At(pos);
            if (ch != ' ')
            {
                cmd += ch;
            }
            else
            {
                data = text.Substring(pos + 1);
                break;
            }
            pos++;
        }
        if (cmd.Compare("a") == 0)
            type = AB::GameProtocol::CommandTypeChatGeneral;
        else if (cmd.Compare("g") == 0)
            type = AB::GameProtocol::CommandTypeChatGuild;
        else if (cmd.Compare("p") == 0)
            type = AB::GameProtocol::CommandTypeChatParty;
        else if (cmd.Compare("trade") == 0)
            type = AB::GameProtocol::CommandTypeChatTrade;
        else if (cmd.Compare("w") == 0)
            type = AB::GameProtocol::CommandTypeChatWhisper;
        else if (cmd.Compare("roll") == 0)
            type = AB::GameProtocol::CommandTypeRoll;
        else if (cmd.Compare("mail") == 0)
            type = AB::GameProtocol::CommandTypeMailSend;
        else if (cmd.Compare("inbox") == 0)
            type = AB::GameProtocol::CommandTypeMailInbox;
        else if (cmd.Compare("read") == 0)
            type = AB::GameProtocol::CommandTypeMailRead;
        else if (cmd.Compare("delete") == 0)
            type = AB::GameProtocol::CommandTypeMailDelete;

        else if (cmd.Compare("age") == 0)
            type = AB::GameProtocol::CommandTypeAge;
        else if (cmd.Compare("deaths") == 0)
            type = AB::GameProtocol::CommandTypeDeaths;
        else if (cmd.Compare("health") == 0)
            type = AB::GameProtocol::CommandTypeHealth;
        else if (cmd.Compare("ip") == 0)
            type = AB::GameProtocol::CommandTypeIp;
        else if (cmd.Compare("id") == 0)
            type = AB::GameProtocol::CommandTypeServerId;
        else if (cmd.Compare("help") == 0)
            type = AB::GameProtocol::CommandTypeHelp;
    }
    else
    {
        switch (defChannel)
        {
        case AB::GameProtocol::ChatChannelGuild:
            type = AB::GameProtocol::CommandTypeChatGuild;
            data = text;
            break;
        case AB::GameProtocol::ChatChannelParty:
            type = AB::GameProtocol::CommandTypeChatParty;
            data = text;
            break;
        case AB::GameProtocol::ChatChannelTrade:
            type = AB::GameProtocol::CommandTypeChatTrade;
            data = text;
            break;
        case AB::GameProtocol::ChatChannelWhisper:
        {
            type = AB::GameProtocol::CommandTypeChatWhisper;
            LineEdit* nameEdit = dynamic_cast<LineEdit*>(GetChild("WhisperChatNameEdit", true));
            String name = nameEdit->GetText();
            data = name + ", " + text;
            break;
        }
        default:
            type = AB::GameProtocol::CommandTypeChatGeneral;
            data = text;
            break;
        }
    }

    switch (type)
    {
    case AB::GameProtocol::CommandTypeHelp:
        AddLine("Available commands:", "ChatLogServerInfoText");
        AddLine("  /a <message>: General chat", "ChatLogServerInfoText");
        AddLine("  /g <message>: Guild chat", "ChatLogServerInfoText");
        AddLine("  /party <message>: Party chat", "ChatLogServerInfoText");
        AddLine("  /g <message>: Guild chat", "ChatLogServerInfoText");
        AddLine("  /trade <message>: Trade chat", "ChatLogServerInfoText");
        AddLine("  /w <name>, <message>: Whisper to <name> a <message>", "ChatLogServerInfoText");
        AddLine("  /mail <name>, [<subject>:] <message>: Send mail to <name> with <message>", "ChatLogServerInfoText");
        AddLine("  /inbox: Show your mail inbox", "ChatLogServerInfoText");
        AddLine("  /read <number>: Read mail with <number>", "ChatLogServerInfoText");
        AddLine("  /delete <number | all>: Delete mail with <number> or all", "ChatLogServerInfoText");
        AddLine("  /roll <number>: Rolls a <number>-sided die (2-100 sides)", "ChatLogServerInfoText");
        AddLine("  /age: Show Character age", "ChatLogServerInfoText");
        AddLine("  /ip: Show server IP", "ChatLogServerInfoText");
        AddLine("  /help: Show this help", "ChatLogServerInfoText");
        break;
    case AB::GameProtocol::CommandTypeIp:
    {
        FwClient* client = context_->GetSubsystem<FwClient>();
        uint32_t ip = client->GetIp();
        char buffer[20];
        sprintf_s(buffer, 20, "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        String sIp(buffer);
        AddLine(sIp, "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandTypeMailInbox:
    {
        FwClient* client = context_->GetSubsystem<FwClient>();
        client->GetMailHeaders();
        break;
    }
    case AB::GameProtocol::CommandTypeMailRead:
    case AB::GameProtocol::CommandTypeMailDelete:
    {
        unsigned p = data.Find(' ');
        String sIndex = data.Substring(p + 1).Trimmed();
        if (!sIndex.Empty())
        {
            FwClient* client = context_->GetSubsystem<FwClient>();
            const std::vector<AB::Entities::MailHeader>& headers = client->GetCurrentMailHeaders();
            if (sIndex.Compare("all") == 0 && (type == AB::GameProtocol::CommandTypeMailDelete))
            {
                client->DeleteMail("all");
            }
            else
            {
                int index = atoi(sIndex.CString());
                if (index > 0 && index <= headers.size())
                {
                    if (type == AB::GameProtocol::CommandTypeMailRead)
                        client->ReadMail(headers[index - 1].uuid);
                    else if (type == AB::GameProtocol::CommandTypeMailDelete)
                        client->DeleteMail(headers[index - 1].uuid);
                }
            }
        }
        break;
    }
    case AB::GameProtocol::CommandTypeUnknown:
        AddLine("Unknown command", "ChatLogServerInfoText");
        break;
    default:
    {
        FwClient* client = context_->GetSubsystem<FwClient>();
        client->Command(type, data.Substring(0, MAX_CHAT_MESSAGE));
        break;
    }
    }
}

void ChatWindow::HandleEditFocused(StringHash eventType, VariantMap& eventData)
{
    UnsubscribeFromEvent(E_KEYDOWN);
}

void ChatWindow::HandleEditDefocused(StringHash eventType, VariantMap& eventData)
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ChatWindow, HandleKeyDown));
}

void ChatWindow::HandleTextFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace TextFinished;

    String line = eventData[P_TEXT].GetString();
    LineEdit* sender = static_cast<LineEdit*>(eventData[P_ELEMENT].GetPtr());
    if (!line.Empty())
    {
        AB::GameProtocol::ChatMessageChannel channel =
            static_cast<AB::GameProtocol::ChatMessageChannel>(sender->GetVar("Channel").GetInt());
        ParseChatCommand(line, channel);
    }
    sender->SetText("");
    sender->SetFocus(false);
}

void ChatWindow::AddLine(const String& text, const String& style)
{
    Text* txt = chatLog_->CreateChild<Text>();

    txt->SetText(text);
    txt->SetStyle(style);
    txt->EnableLayoutUpdate();
    txt->SetMaxWidth(chatLog_->GetWidth() - 8);
    txt->SetWordwrap(true);
    txt->UpdateLayout();
    chatLog_->AddItem(txt);
    chatLog_->EnsureItemVisibility(txt);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}

void ChatWindow::AddLine(const String& name, const String& text, const String& style)
{
    Text* txt = chatLog_->CreateChild<Text>();

    txt->SetVar("Name", name);
    txt->SetText(text);
    txt->SetStyle(style);
    txt->EnableLayoutUpdate();
    txt->SetMaxWidth(chatLog_->GetWidth() - 8);
    txt->SetWordwrap(true);
    txt->UpdateLayout();
    SubscribeToEvent(txt, E_CLICK, URHO3D_HANDLER(ChatWindow, HandleNameClicked));

    chatLog_->AddItem(txt);
    chatLog_->EnsureItemVisibility(txt);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}

void ChatWindow::AddLine(uint32_t id, const String& name, const String& text,
    const String& style, const String& style2 /* = String::EMPTY */,
    AB::GameProtocol::ChatMessageChannel channel /* = AB::GameProtocol::ChatChannelUnknown */)
{
    Text* nameText = chatLog_->CreateChild<Text>();
    nameText->SetVar("ID", id);
    nameText->SetVar("Name", name);
    nameText->SetVar("Channel", channel);
    if (!channel == AB::GameProtocol::ChatChannelWhisper)
        nameText->SetText(name + ":");
    else
        nameText->SetText("{" + name + "}");
    SubscribeToEvent(nameText, E_CLICK, URHO3D_HANDLER(ChatWindow, HandleNameClicked));
    nameText->SetStyle(style);
    nameText->EnableLayoutUpdate();
    nameText->SetMaxWidth(chatLog_->GetWidth() - 8);
    Text* textText = nameText->CreateChild<Text>();
    textText->SetPosition(IntVector2(nameText->GetWidth() + 10, 0));
    textText->SetText(text);
    if (!style2.Empty())
        textText->SetStyle(style2);
    else
        textText->SetStyle(style);
    textText->SetMaxWidth(chatLog_->GetWidth() - textText->GetPosition().x_);
    textText->SetWordwrap(true);
    nameText->SetMinHeight(textText->GetHeight());
    textText->EnableLayoutUpdate();
    textText->UpdateLayout();

    chatLog_->AddItem(nameText);
    chatLog_->EnsureItemVisibility(nameText);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}

void ChatWindow::AddChatLine(uint32_t senderId, const String& name,
    const String& text, AB::GameProtocol::ChatMessageChannel channel)
{
    String style;
    String testStyle = "ChatLogChatText";
    switch (channel)
    {
    case AB::GameProtocol::ChatChannelGeneral:
        style = "ChatLogGeneralChatText";
        break;
    case AB::GameProtocol::ChatChannelGuild:
        style = "ChatLogGuildChatText";
        break;
    case AB::GameProtocol::ChatChannelParty:
        style = "ChatLogPartyChatText";
        break;
    case AB::GameProtocol::ChatChannelWhisper:
        style = "ChatLogWhisperChatText";
        break;
    case AB::GameProtocol::ChatChannelTrade:
        style = "ChatLogTradeChatText";
        testStyle = "ChatLogTradeChatText";
        break;
    default:
        style = "ChatLogChatText";
        break;
    }
    AddLine(senderId, name, text, style, testStyle, channel);
}
