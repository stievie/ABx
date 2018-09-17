#include "stdafx.h"
#include "ChatWindow.h"
#include "FwClient.h"
#include "AbEvents.h"
#include "Utils.h"
#include <TimeUtils.h>
#include <Mustache/mustache.hpp>
#include "Shortcuts.h"
#include "Options.h"
#include "LevelManager.h"
#include "Player.h"

#include <Urho3D/DebugNew.h>

ChatWindow::ChatWindow(Context* context) :
    UIElement(context),
    tabIndexWhisper_(-1),
    firstStart_(true)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/ChatWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);
    SetName("ChatWindow");

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

    SubscribeToEvent(AbEvents::E_SCREENSHOTTAKEN, URHO3D_HANDLER(ChatWindow, HandleScreenshotTaken));
    SubscribeToEvent(AbEvents::E_SERVERMESSAGE, URHO3D_HANDLER(ChatWindow, HandleServerMessage));
    SubscribeToEvent(AbEvents::E_CHATMESSAGE, URHO3D_HANDLER(ChatWindow, HandleChatMessage));
    SubscribeToEvent(AbEvents::E_SC_CHATGENERAL, URHO3D_HANDLER(ChatWindow, HandleShortcutChatGeneral));
    SubscribeToEvent(AbEvents::E_SC_CHATGUILD, URHO3D_HANDLER(ChatWindow, HandleShortcutChatGuild));
    SubscribeToEvent(AbEvents::E_SC_CHATPARTY, URHO3D_HANDLER(ChatWindow, HandleShortcutChatParty));
    SubscribeToEvent(AbEvents::E_SC_CHATTRADE, URHO3D_HANDLER(ChatWindow, HandleShortcutChatTrade));
    SubscribeToEvent(AbEvents::E_SC_CHATWHISPER, URHO3D_HANDLER(ChatWindow, HandleShortcutChatWhisper));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ChatWindow, HandleKeyDown));

    SetAlignment(HA_LEFT, VA_BOTTOM);
}

void ChatWindow::FocusEdit()
{
    LineEdit* edit = GetActiveLineEdit();
    if (edit && !edit->HasFocus())
        edit->SetFocus(true);
}

void ChatWindow::CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatMessageChannel channel)
{
    static const IntVector2 tabSize(90, 20);
    static const IntVector2 tabBodySize(500, 20);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    TabElement *tabElement = tabs->CreateTab(tabSize, tabBodySize);
    tabElement->tabText_->SetFont(cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf"), 10);
    switch (channel)
    {
    case AB::GameProtocol::ChatChannelGeneral:
        tabElement->tabText_->SetText(scs->GetCaption(AbEvents::E_SC_CHATGENERAL, "General", true));
        break;
    case AB::GameProtocol::ChatChannelParty:
        tabElement->tabText_->SetText(scs->GetCaption(AbEvents::E_SC_CHATPARTY, "Party", true));
        break;
    case AB::GameProtocol::ChatChannelGuild:
        tabElement->tabText_->SetText(scs->GetCaption(AbEvents::E_SC_CHATGUILD, "Guild", true));
        break;
    case AB::GameProtocol::ChatChannelTrade:
        tabElement->tabText_->SetText(scs->GetCaption(AbEvents::E_SC_CHATTRADE, "Trade", true));
        break;
    case AB::GameProtocol::ChatChannelWhisper:
        tabElement->tabText_->SetText(scs->GetCaption(AbEvents::E_SC_CHATWHISPER, "Whisper", true));
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
        SubscribeToEvent(nameEdit, E_FOCUSED, [](StringHash, VariantMap& eventData)
        {
            using namespace Focused;
            UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
            elem->SetOpacity(1.0f);
        });
        SubscribeToEvent(nameEdit, E_DEFOCUSED, [](StringHash, VariantMap& eventData)
        {
            using namespace Defocused;
            UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
            elem->SetOpacity(0.7f);
        });
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

void ChatWindow::HandleServerMessage(StringHash, VariantMap& eventData)
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
    ss << "You have played this character for ";
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
    const String& data = eventData[P_DATA].GetString();
    AddLine("Player " + data + " is not online.", "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerGotMessage(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();
    const String& data = eventData[P_DATA].GetString();
    AddLine("{" + name + "} " + data, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageNewMail(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& count = eventData[P_DATA].GetString();
    kainjow::mustache::mustache tpl{ "You got a new mail, total {{count}} mail(s)." };
    kainjow::mustache::data data;
    data.set("count", std::string(count.CString(), count.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");

    VariantMap& eData = GetEventDataMap();
    using namespace AbEvents::NewMail;
    eData[P_COUNT] = atoi(count.CString());
    SendEvent(AbEvents::E_NEWMAIL, eData);
}

void ChatWindow::HandleServerMessageMailSent(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();
    kainjow::mustache::mustache tpl{ "Mail to {{recipient}} was sent." };
    kainjow::mustache::data data;
    data.set("recipient", std::string(name.CString(), name.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailNotSent(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();
    kainjow::mustache::mustache tpl{ "Mail to {{recipient}} was not sent. Please check the name, or the mail box is full." };
    kainjow::mustache::data data;
    data.set("recipient", std::string(name.CString(), name.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailboxFull(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& count = eventData[P_DATA].GetString();
    kainjow::mustache::mustache tpl{ "Your mailbox is full! You have {{count}} mails. Please delete some, so people are able to send you mails." };
    kainjow::mustache::data data;
    data.set("count", std::string(count.CString(), count.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailDeleted(VariantMap&)
{
    AddLine("The mail was deleted.", "ChatLogServerInfoText");
}

void ChatWindow::HandleChatMessage(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ChatMessage;
    AB::GameProtocol::ChatMessageChannel channel =
        static_cast<AB::GameProtocol::ChatMessageChannel>(eventData[P_MESSAGETYPE].GetInt());
    const String& message = eventData[P_DATA].GetString();
    const String& sender = eventData[P_SENDER].GetString();
    uint32_t senderId = static_cast<uint32_t>(eventData[P_SENDERID].GetInt());
    AddChatLine(senderId, sender, message, channel);
}

void ChatWindow::HandleTabSelected(StringHash, VariantMap& eventData)
{
    using namespace TabSelected;
    int idx = eventData[P_INDEX].GetInt();
    LineEdit* edit = GetLineEdit(idx);
    if (edit)
        edit->SetFocus(true);
}

void ChatWindow::HandleKeyDown(StringHash, VariantMap& eventData)
{
    using namespace KeyDown;

    int key = eventData[P_KEY].GetInt();
    if (key == KEY_RETURN)
        FocusEdit();
}

void ChatWindow::HandleNameClicked(StringHash, VariantMap& eventData)
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

void ChatWindow::HandleShortcutChatGeneral(StringHash, VariantMap&)
{
    tabgroup_->SetSelectedIndex(0);
    FocusEdit();
}

void ChatWindow::HandleShortcutChatGuild(StringHash, VariantMap&)
{
    tabgroup_->SetSelectedIndex(1);
    FocusEdit();
}

void ChatWindow::HandleServerMessageServerId(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& id = eventData[P_DATA].GetString();
    AddLine(id, "ChatLogServerInfoText");
}

void ChatWindow::HandleShortcutChatParty(StringHash, VariantMap&)
{
    tabgroup_->SetSelectedIndex(2);
    FocusEdit();
}

void ChatWindow::HandleShortcutChatTrade(StringHash, VariantMap&)
{
    tabgroup_->SetSelectedIndex(3);
    FocusEdit();
}

void ChatWindow::HandleShortcutChatWhisper(StringHash, VariantMap&)
{
    tabgroup_->SetSelectedIndex(tabIndexWhisper_);
    FocusEdit();
}

bool ChatWindow::ParseChatCommand(const String& text, AB::GameProtocol::ChatMessageChannel defChannel)
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
        else if (cmd.Compare("sit") == 0)
            type = AB::GameProtocol::CommandTypeSit;
        else if (cmd.Compare("stand") == 0)
            type = AB::GameProtocol::CommandTypeStand;
        else if (cmd.Compare("cry") == 0)
            type = AB::GameProtocol::CommandTypeCry;

        else if (cmd.Compare("age") == 0)
            type = AB::GameProtocol::CommandTypeAge;
        else if (cmd.Compare("deaths") == 0)
            type = AB::GameProtocol::CommandTypeDeaths;
        else if (cmd.Compare("health") == 0)
            type = AB::GameProtocol::CommandTypeHealth;
        else if (cmd.Compare("ip") == 0)
            type = AB::GameProtocol::CommandTypeIp;
        else if (cmd.Compare("ip") == 0)
            type = AB::GameProtocol::CommandTypeIp;
        else if (cmd.Compare("prefpath") == 0)
            type = AB::GameProtocol::CommandTypePrefPath;
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
            String name = nameEdit->GetText().Trimmed();
            if (name.Empty())
            {
                nameEdit->SetFocus(true);
                return false;
            }
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
        AddLine("  /roll <number>: Rolls a <number>-sided die (2-100 sides)", "ChatLogServerInfoText");
        AddLine("  /age: Show Character age", "ChatLogServerInfoText");
        AddLine("  /ip: Show server IP", "ChatLogServerInfoText");
        AddLine("  /prefpath: Show preferences path", "ChatLogServerInfoText");
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
    case AB::GameProtocol::CommandTypePrefPath:
    {
        AddLine(Options::GetPrefPath(), "ChatLogServerInfoText");
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
    return true;
}

void ChatWindow::TrimLines()
{
    while (chatLog_->GetNumItems() > MAX_LINES)
        chatLog_->RemoveItem((unsigned)0);
}

void ChatWindow::HandleScreenshotTaken(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ScreenshotTaken;
    const String& file = eventData[P_FILENAME].GetString();

    kainjow::mustache::mustache tpl{ "Screenshot saved to {{file}}" };
    kainjow::mustache::data data;
    data.set("file", std::string(file.CString()));
    std::string t = tpl.render(data);

    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleEditFocused(StringHash, VariantMap& eventData)
{
    UnsubscribeFromEvent(E_KEYDOWN);
    using namespace Focused;
    UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    elem->SetOpacity(1.0f);
}

void ChatWindow::HandleEditDefocused(StringHash, VariantMap& eventData)
{
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ChatWindow, HandleKeyDown));
    using namespace Defocused;
    UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
    elem->SetOpacity(0.7f);
}

void ChatWindow::HandleTextFinished(StringHash, VariantMap& eventData)
{
    using namespace TextFinished;

    const String& line = eventData[P_TEXT].GetString();
    LineEdit* sender = static_cast<LineEdit*>(eventData[P_ELEMENT].GetPtr());
    if (!line.Empty())
    {
        AB::GameProtocol::ChatMessageChannel channel =
            static_cast<AB::GameProtocol::ChatMessageChannel>(sender->GetVar("Channel").GetInt());
        if (ParseChatCommand(line, channel))
        {
            sender->SetText("");
        }
    }
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
    TrimLines();
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
    TrimLines();
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
    if (channel != AB::GameProtocol::ChatChannelWhisper)
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
    TrimLines();
    chatLog_->EnsureItemVisibility(nameText);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}

void ChatWindow::AddChatLine(uint32_t senderId, const String& name,
    const String& text, AB::GameProtocol::ChatMessageChannel channel)
{
    String style;
    String textStyle = "ChatLogChatText";
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
        textStyle = "ChatLogTradeChatText";
        break;
    default:
        style = "ChatLogChatText";
        break;
    }
    AddLine(senderId, name, text, style, textStyle, channel);
}

void ChatWindow::SayHello(Player* player)
{
    if (firstStart_)
    {
        kainjow::mustache::mustache tpl{ "Hi {{name}}, type /help for available commands." };
        kainjow::mustache::data data;
        if (player)
            data.set("name", std::string(player->name_.CString(), player->name_.Length()));
        else
            data.set("name", "Unknown Soldier");
        std::string t = tpl.render(data);
        AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
        firstStart_ = false;
    }
}
