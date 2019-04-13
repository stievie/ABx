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
#include "SkillManager.h"
#include "ItemsCache.h"

#include <Urho3D/DebugNew.h>

const HashMap<String, AB::GameProtocol::CommandTypes> ChatWindow::CHAT_COMMANDS = {
    { "a", AB::GameProtocol::CommandTypeChatGeneral },
    { "g", AB::GameProtocol::CommandTypeChatGuild },
    { "p", AB::GameProtocol::CommandTypeChatParty },
    { "trade", AB::GameProtocol::CommandTypeChatTrade },
    { "w", AB::GameProtocol::CommandTypeChatWhisper },
    { "roll", AB::GameProtocol::CommandTypeRoll },
    { "sit", AB::GameProtocol::CommandTypeSit },
    { "stand", AB::GameProtocol::CommandTypeStand },
    { "cry", AB::GameProtocol::CommandTypeCry },
    { "age", AB::GameProtocol::CommandTypeAge },
    { "deaths", AB::GameProtocol::CommandTypeDeaths },
    { "hp", AB::GameProtocol::CommandTypeHealth },
    { "pos", AB::GameProtocol::CommandTypePos },
    { "ip", AB::GameProtocol::CommandTypeIp },
    { "id", AB::GameProtocol::CommandTypeServerId },
    { "prefpath", AB::GameProtocol::CommandTypePrefPath },
    { "taunt", AB::GameProtocol::CommandTypeTaunt },
    { "ponder", AB::GameProtocol::CommandTypePonder },
    { "wave", AB::GameProtocol::CommandTypeWave },
    { "laugh", AB::GameProtocol::CommandTypeLaugh },
    { "resign", AB::GameProtocol::CommandTypeResign },

    { "help", AB::GameProtocol::CommandTypeHelp }
};

ChatWindow::ChatWindow(Context* context) :
    UIElement(context),
    historyRows_(20),
    tabIndexWhisper_(-1),
    firstStart_(true),
    visibleGeneral_(true),
    visibleGuild_(true),
    visibleParty_(true),
    visibleTrade_(true),
    visibleWhisper_(true)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile *chatFile = cache->GetResource<XMLFile>("UI/ChatWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);
    SetName("ChatWindow");

    // Set self to same size as the window so align works
    Window* wnd = dynamic_cast<Window*>(GetChild("ChatWindow", true));
    wnd->SetPosition(0, 0);
    SetSize(wnd->GetSize());
    wnd->SetBringToBack(false);
    wnd->SetPriority(200);

    // Chat filter
    UIElement* filterContainer = wnd->GetChild("ChatFilter", false);
    {
        CheckBox* filterCheck = dynamic_cast<CheckBox*>(filterContainer->GetChild("ChatFilterGeneral", true));
        filterCheck->SetChecked(visibleGeneral_);
        filterCheck->SetVar("Channel", AB::GameProtocol::ChatChannelGeneral);
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = dynamic_cast<CheckBox*>(filterContainer->GetChild("ChatFilterGuild", true));
        filterCheck->SetChecked(visibleGuild_);
        filterCheck->SetVar("Channel", AB::GameProtocol::ChatChannelGuild);
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = dynamic_cast<CheckBox*>(filterContainer->GetChild("ChatFilterParty", true));
        filterCheck->SetChecked(visibleParty_);
        filterCheck->SetVar("Channel", AB::GameProtocol::ChatChannelParty);
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = dynamic_cast<CheckBox*>(filterContainer->GetChild("ChatFilterTrade", true));
        filterCheck->SetChecked(visibleTrade_);
        filterCheck->SetVar("Channel", AB::GameProtocol::ChatChannelTrade);
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = dynamic_cast<CheckBox*>(filterContainer->GetChild("ChatFilterWhisper", true));
        filterCheck->SetChecked(visibleWhisper_);
        filterCheck->SetVar("Channel", AB::GameProtocol::ChatChannelWhisper);
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }

    UIElement* container = dynamic_cast<UIElement*>(wnd->GetChild("ChatTextContainer", false));
    tabgroup_ = container->CreateChild<TabGroup>();
    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_BOTTOM);
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
    SubscribeToEvent(AbEvents::E_OBJECTPINGTARGET, URHO3D_HANDLER(ChatWindow, HandleTargetPinged));
    SubscribeToEvent(AbEvents::E_PARTYRESIGNED, URHO3D_HANDLER(ChatWindow, HandlePartyResigned));
    SubscribeToEvent(AbEvents::E_PARTYDEFEATED, URHO3D_HANDLER(ChatWindow, HandlePartyDefeated));
    SubscribeToEvent(AbEvents::E_OBJECTITEMDROPPED, URHO3D_HANDLER(ChatWindow, HandleItemDropped));
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
    SubscribeToEvent(edit, E_UNHANDLEDKEY, URHO3D_HANDLER(ChatWindow, HandleEditKey));

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
    case AB::GameProtocol::ServerMessageTypePos:
        HandleServerMessagePos(eventData);
        break;
    case AB::GameProtocol::ServerMessageTypeHp:
        HandleServerMessageHp(eventData);
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
    case AB::GameProtocol::ServerMessageTypePlayerResigned:
        HandleServerMessagePlayerResigned(eventData);
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
    const String& message = eventData[P_DATA].GetString();
    AddLine(message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageRoll(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    const String& sender = eventData[P_SENDER].GetString();
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
    const String& message = eventData[P_DATA].GetString();
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

void ChatWindow::HandleServerMessageHp(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    kainjow::mustache::mustache tpl{ "Health {{currHp}}/{{maxHp}}, Energy {{currE}}/{{maxE}}" };
    kainjow::mustache::data data;
    auto parts = message.Split('|');
    if (parts.Size() > 0)
    {
        unsigned p = parts[0].Find(":");
        String currHp = parts[0].Substring(0, p);
        String maxHp = parts[0].Substring(p + 1);
        data.set("currHp", std::string(currHp.CString(), currHp.Length()));
        data.set("maxHp", std::string(maxHp.CString(), maxHp.Length()));
    }
    if (parts.Size() > 1)
    {
        unsigned p = parts[1].Find(":");
        String currE = parts[1].Substring(0, p);
        String maxE = parts[1].Substring(p + 1);
        data.set("currE", std::string(currE.CString(), currE.Length()));
        data.set("maxE", std::string(maxE.CString(), maxE.Length()));
    }
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePos(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    AddLine(message, "ChatLogServerInfoText");
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

void ChatWindow::HandleServerMessagePlayerResigned(VariantMap& eventData)
{
    using namespace AbEvents::ServerMessage;
    const String& resigner = eventData[P_SENDER].GetString();
    kainjow::mustache::mustache tpl{ "{{name}} has resigned" };
    kainjow::mustache::data data;
    data.set("name", std::string(resigner.CString(), resigner.Length()));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
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

void ChatWindow::HandlePartyResigned(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::PartyResigned;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();

    kainjow::mustache::mustache tpl{ "Party {{id}} has resigned" };
    kainjow::mustache::data data;
    data.set("id", std::to_string(partyId));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandlePartyDefeated(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::PartyDefeated;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();

    kainjow::mustache::mustache tpl{ "Party {{id}} was defeated" };
    kainjow::mustache::data data;
    data.set("id", std::to_string(partyId));
    std::string t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
}

void ChatWindow::HandleTargetPinged(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ObjectPingTarget;
/*
    URHO3D_PARAM(P_OBJECTID, ObjectId);
    URHO3D_PARAM(P_TARGETID, TargetId);
    URHO3D_PARAM(P_SKILLINDEX, SkillIndex);
*/
    String message = "I am";
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    AB::GameProtocol::ObjectCallType type = static_cast<AB::GameProtocol::ObjectCallType>(eventData[P_CALLTTYPE].GetUInt());
    int skillIndex = eventData[P_SKILLINDEX].GetUInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    Actor* pinger = dynamic_cast<Actor*>(lm->GetObjectById(objectId).Get());
    Actor* target = dynamic_cast<Actor*>(lm->GetObjectById(targetId).Get());

    switch (type)
    {
    case AB::GameProtocol::ObjectCallTypeFollow:
        if (!target)
            return;
        message += " following " + target->name_;
        break;
    case AB::GameProtocol::ObjectCallTypeAttack:
        if (!target)
            return;
        message += " attacking " + target->name_;
        break;
    case AB::GameProtocol::ObjectCallTypeUseSkill:
    {
        if (skillIndex <= 0)
            return;
        const AB::Entities::Skill* skill = GetSubsystem<SkillManager>()->GetSkillByIndex(pinger->skills_[skillIndex - 1]);
        message += " using " + String(skill->name.c_str());
        if (target)
            message += " on " + target->name_;
    }
    }
    AddChatLine(objectId, pinger->name_, message, AB::GameProtocol::ChatChannelParty);
    pinger->ShowSpeechBubble(message);
}

void ChatWindow::HandleItemDropped(StringHash, VariantMap& eventData)
{
    using namespace AbEvents::ObjectItemDropped;
    uint32_t dropperId = eventData[P_OBJECTID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    uint32_t itemIndex = eventData[P_ITEMINDEX].GetUInt();
    uint32_t count = eventData[P_COUNT].GetUInt();

    LevelManager* lm = GetSubsystem<LevelManager>();
    Actor* dropper = dynamic_cast<Actor*>(lm->GetObjectById(dropperId).Get());
    Actor* target = dynamic_cast<Actor*>(lm->GetObjectById(targetId).Get());

    ItemsCache* items = GetSubsystem<ItemsCache>();
    // Item may not be spawned yet
    Item* item = items->Get(itemIndex);

    kainjow::mustache::mustache tplCount{ "{{dropper}} dropped {{count}} {{item}} for {{target}}" };
    kainjow::mustache::mustache tpl{ "{{dropper}} dropped {{item}} for {{target}}" };
    kainjow::mustache::data data;
    data.set("dropper", std::string(dropper->name_.CString()));
    if (item)
        data.set("item", std::string(item->name_.CString()));
    else
    {
        data.set("item", std::to_string(itemIndex));
    }
    data.set("target", std::string(target->name_.CString()));
    std::string t;
    if (count > 1)
    {
        data.set("count", std::to_string(count));
        t = tplCount.render(data);
    }
    else
        t = tpl.render(data);
    AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
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
        auto cmdIt = CHAT_COMMANDS.Find(cmd);
        if (cmdIt != CHAT_COMMANDS.End())
            type = (*cmdIt).second_;
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
        AddLine("  /resign: Resign", "ChatLogServerInfoText");
        AddLine("  /age: Show Character age", "ChatLogServerInfoText");
        AddLine("  /hp: Show health points and energy", "ChatLogServerInfoText");
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
        chatLog_->RemoveItem(0u);
}

void ChatWindow::UpdateVisibleItems()
{
    for (unsigned i = 0; i < chatLog_->GetNumItems(); ++i)
    {
        UIElement* elem = chatLog_->GetItem(i);
        AB::GameProtocol::ChatMessageChannel channel = static_cast<AB::GameProtocol::ChatMessageChannel>(elem->GetVar("Channel").GetUInt());
        switch (channel)
        {
        case AB::GameProtocol::ChatChannelGeneral:
            elem->SetVisible(visibleGeneral_);
            break;
        case AB::GameProtocol::ChatChannelGuild:
            elem->SetVisible(visibleGuild_);
            break;
        case AB::GameProtocol::ChatChannelParty:
            elem->SetVisible(visibleParty_);
            break;
        case AB::GameProtocol::ChatChannelTrade:
            elem->SetVisible(visibleTrade_);
            break;
        case AB::GameProtocol::ChatChannelWhisper:
            elem->SetVisible(visibleWhisper_);
            break;
        }
    }
}

void ChatWindow::HandleFilterClick(StringHash, VariantMap& eventData)
{
    using namespace Toggled;
    CheckBox* sender = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());
    bool state = eventData[P_STATE].GetBool();
    AB::GameProtocol::ChatMessageChannel channel = static_cast<AB::GameProtocol::ChatMessageChannel>(sender->GetVar("Channel").GetUInt());
    switch (channel)
    {
    case AB::GameProtocol::ChatChannelGeneral:
        visibleGeneral_ = state;
        break;
    case AB::GameProtocol::ChatChannelGuild:
        visibleGuild_ = state;
        break;
    case AB::GameProtocol::ChatChannelParty:
        visibleParty_ = state;
        break;
    case AB::GameProtocol::ChatChannelTrade:
        visibleTrade_ = state;
        break;
    case AB::GameProtocol::ChatChannelWhisper:
        visibleWhisper_ = state;
        break;
    default:
        return;
    }
    UpdateVisibleItems();
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
            // Make sure the line isn't the same as the last one
            if (history_.Empty() || line != history_.Back())
            {
                auto it = history_.Find(line);
                if (it != history_.End())
                    history_.Erase(it);

                // Store to history, then clear the lineedit
                history_.Push(line);
                if (history_.Size() > historyRows_)
                    history_.Erase(history_.Begin());
            }
            historyPosition_ = history_.Size(); // Reset
            sender->SetText(String::EMPTY);
            currentRow_.Clear();
        }
    }
    sender->SetFocus(false);
}

void ChatWindow::HandleEditKey(StringHash, VariantMap& eventData)
{
    if (!historyRows_)
        return;

    using namespace UnhandledKey;

    bool changed = false;

    LineEdit* edit = dynamic_cast<LineEdit*>(eventData[P_ELEMENT].GetPtr());

    switch (eventData[P_KEY].GetInt())
    {
    case KEY_UP:
        if (historyPosition_ > 0)
        {
            --historyPosition_;
            changed = true;
        }
        break;

    case KEY_DOWN:
        // If history options left
        if (historyPosition_ < history_.Size())
        {
            // Use the next option
            ++historyPosition_;
            changed = true;
        }
        break;

    default:
        break;
    }

    if (changed)
    {
        // Set text to history option
        if (historyPosition_ < history_.Size())
            edit->SetText(history_[historyPosition_]);
        else // restore the original line value before it was set to history values
        {
            edit->SetText(currentRow_);
        }
    }

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
    switch (channel)
    {
    case AB::GameProtocol::ChatChannelGeneral:
        nameText->SetVisible(visibleGeneral_);
        break;
    case AB::GameProtocol::ChatChannelGuild:
        nameText->SetVisible(visibleGuild_);
        break;
    case AB::GameProtocol::ChatChannelParty:
        nameText->SetVisible(visibleParty_);
        break;
    case AB::GameProtocol::ChatChannelTrade:
        nameText->SetVisible(visibleTrade_);
        break;
    case AB::GameProtocol::ChatChannelWhisper:
        nameText->SetVisible(visibleWhisper_);
        break;
    default:
        nameText->SetVisible(true);
        break;
    }
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
    if (firstStart_ && player)
    {
        kainjow::mustache::mustache tpl{ "Hello {{name}}, type /help for available commands." };
        kainjow::mustache::data data;
        data.set("name", std::string(player->name_.CString(), player->name_.Length()));
        std::string t = tpl.render(data);
        AddLine(String(t.c_str(), (unsigned)t.size()), "ChatLogServerInfoText");
        firstStart_ = false;
    }
}
