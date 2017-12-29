#include "stdafx.h"
#include "ChatWindow.h"
#include <AB/ProtocolCodes.h>
#include "FwClient.h"
#include "AbEvents.h"
#include "Utils.h"

#include <Urho3D/DebugNew.h>

ChatWindow::ChatWindow(Context* context) :
    UIElement(context)
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

    chatLog_ = dynamic_cast<ListView*>(GetChild("ChatLog", true));
    chatEdit_ = dynamic_cast<LineEdit*>(GetChild("ChatEdit", true));
    chatEdit_->SetStyle("ChatLineEdit");

    SubscribeToEvent(chatEdit_, E_TEXTCHANGED, URHO3D_HANDLER(ChatWindow, HandleTextChanged));
    SubscribeToEvent(chatEdit_, E_TEXTFINISHED, URHO3D_HANDLER(ChatWindow, HandleTextFinished));
    SubscribeToEvent(chatEdit_, E_UNHANDLEDKEY, URHO3D_HANDLER(ChatWindow, HandleChatEditKey));
    SubscribeToEvent(chatEdit_, E_HOVERBEGIN, URHO3D_HANDLER(ChatWindow, HandleHoverBegin));
    SubscribeToEvent(chatEdit_, E_HOVEREND, URHO3D_HANDLER(ChatWindow, HandleHoverEnd));

    SubscribeToEvent(chatLog_, E_HOVERBEGIN, URHO3D_HANDLER(ChatWindow, HandleHoverBegin));
    SubscribeToEvent(chatLog_, E_HOVEREND, URHO3D_HANDLER(ChatWindow, HandleHoverEnd));

    SubscribeToEvent(AbEvents::E_SERVER_MESSAGE, URHO3D_HANDLER(ChatWindow, HandleServerMessage));
}

ChatWindow::~ChatWindow()
{
    UnsubscribeFromAllEvents();
}

void ChatWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<ChatWindow>();
}

void ChatWindow::HandleHoverBegin(StringHash eventType, VariantMap& eventData)
{
    using namespace HoverBegin;
    UIElement* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
}

void ChatWindow::HandleHoverEnd(StringHash eventType, VariantMap& eventData)
{
    using namespace HoverEnd;
    UIElement* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
}

void ChatWindow::HandleServerMessage(StringHash eventType, VariantMap& eventData)
{
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(eventData[AbEvents::ED_MESSAGE_TYPE].GetInt());
    String message = eventData[AbEvents::ED_MESSAGE_DATA].GetString();
    switch (type)
    {
    case AB::GameProtocol::ServerMessageTypeInfo:
        AddLine(message, "ChatLogServerInfoText");
        break;
    case AB::GameProtocol::ServerMessageTypeRoll:
    {
        String sender = eventData[AbEvents::ED_MESSAGE_SENDER].GetString();
        unsigned p = message.Find(":");
        String res = message.Substring(0, p);
        String max = message.Substring(p + 1);
        AddLine(sender + " rolls " + res + " on a " + max + " sided die.", "ChatLogChatText");
        break;
    }
    case AB::GameProtocol::ServerMessageTypeChatGeneral:
    {
        String sender = eventData[AbEvents::ED_MESSAGE_SENDER].GetString();
        AddLine(sender, message, "ChatLogGeneralChatText", "ChatLogChatText");
        break;
    }
    }
}

void ChatWindow::HandleTextChanged(StringHash eventType, VariantMap& eventData)
{

}

void ChatWindow::ParseChatCommand(const String& text)
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
        else if (cmd.Compare("ally") == 0)
            type = AB::GameProtocol::CommandTypeChatAlliance;
        else if (cmd.Compare("trade") == 0)
            type = AB::GameProtocol::CommandTypeChatTrade;
        else if (cmd.Compare("w") == 0)
            type = AB::GameProtocol::CommandTypeChatWhisper;
        else if (cmd.Compare("roll") == 0)
            type = AB::GameProtocol::CommandTypeRoll;

        else if (cmd.Compare("age") == 0)
            type = AB::GameProtocol::CommandTypeAge;
        else if (cmd.Compare("deaths") == 0)
            type = AB::GameProtocol::CommandTypeDeaths;
        else if (cmd.Compare("health") == 0)
            type = AB::GameProtocol::CommandTypeHealth;
        else if (cmd.Compare("ip") == 0)
            type = AB::GameProtocol::CommandTypeIp;
        else if (cmd.Compare("help") == 0)
            type = AB::GameProtocol::CommandTypeHelp;
    }
    else
    {
        type = AB::GameProtocol::CommandTypeChatGeneral;
        data = text;
    }

    switch (type)
    {
    case AB::GameProtocol::CommandTypeHelp:
        AddLine("Available commands:", "ChatLogServerInfoText");
        AddLine("  /a <message>: General chat", "ChatLogServerInfoText");
        AddLine("  /roll <number>: Rolls a <number>-sided die (2-100 sides)", "ChatLogServerInfoText");
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

void ChatWindow::HandleTextFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace TextFinished;

    String line = eventData[P_TEXT].GetString();
    if (!line.Empty())
    {
        ParseChatCommand(line);
    }
    LineEdit* sender = dynamic_cast<LineEdit*>(eventData[P_ELEMENT].GetPtr());
    sender->SetText("");
    sender->SetFocus(false);
}

void ChatWindow::HandleChatEditKey(StringHash eventType, VariantMap& eventData)
{
    using namespace UnhandledKey;
/*    switch (eventData[P_KEY].GetInt())
    {
    } */
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

void ChatWindow::AddLine(const String& name, const String& text,
    const String& style, const String& style2 /* = String::EMPTY */)
{
    Text* nameText = chatLog_->CreateChild<Text>();
    nameText->SetText(name + ":");
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
