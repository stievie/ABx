#include "stdafx.h"
#include "ChatWindow.h"
#include <AB/ProtocolCodes.h>
#include "FwClient.h"
#include "AbEvents.h"

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
    String sender = eventData[AbEvents::ED_MESSAGE_SENDER].GetString();
    String message = eventData[AbEvents::ED_MESSAGE_DATA].GetString();
    AddLine(sender + ": " + message, "ChatLogGeneralChatText");
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
        while (pos < text.Length() - 1)
        {
            char ch = text.At(pos);
            if (ch != ' ')
            {
                cmd += ch;
            }
            else
            {
                data = text.Substring(pos);
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

        else if (cmd.Compare("age") == 0)
            type = AB::GameProtocol::CommandTypeAge;
        else if (cmd.Compare("deaths") == 0)
            type = AB::GameProtocol::CommandTypeDeaths;
        else if (cmd.Compare("health") == 0)
            type = AB::GameProtocol::CommandTypeHealth;
    }
    else
    {
        type = AB::GameProtocol::CommandTypeChatGeneral;
        data = text;
    }

    if (type != AB::GameProtocol::CommandTypeUnknown)
    {
        FwClient* client = context_->GetSubsystem<FwClient>();
        client->Command(type, data);
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
    Text* txt = new Text(context_);
    txt->SetText(text);
    txt->SetStyle(style);
    chatLog_->AddItem(txt);
    chatLog_->EnsureItemVisibility(txt);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}
