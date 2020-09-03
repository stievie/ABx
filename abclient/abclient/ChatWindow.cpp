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

#include "ChatWindow.h"
#include "Conversions.h"
#include "FwClient.h"
#include "Utils.h"
#include <sa/Compiler.h>
#include "Shortcuts.h"
#include "Options.h"
#include "LevelManager.h"
#include "Player.h"
#include "SkillManager.h"
#include "ItemsCache.h"
#include "FormatText.h"
#include "MultiLineEdit.h"
#include <fstream>
#include "ChatFilter.h"
#include <sa/TemplateParser.h>
#include <sa/time.h>

//#include <Urho3D/DebugNew.h>

const HashMap<String, AB::GameProtocol::CommandType> ChatWindow::CHAT_COMMANDS = {
    { "a", AB::GameProtocol::CommandType::ChatGeneral },
    { "g", AB::GameProtocol::CommandType::ChatGuild },
    { "p", AB::GameProtocol::CommandType::ChatParty },
    { "trade", AB::GameProtocol::CommandType::ChatTrade },
    { "w", AB::GameProtocol::CommandType::ChatWhisper },
    { "roll", AB::GameProtocol::CommandType::Roll },
    { "sit", AB::GameProtocol::CommandType::Sit },
    { "stand", AB::GameProtocol::CommandType::Stand },
    { "cry", AB::GameProtocol::CommandType::Cry },
    { "age", AB::GameProtocol::CommandType::Age },
    { "deaths", AB::GameProtocol::CommandType::Deaths },
    { "hp", AB::GameProtocol::CommandType::Health },
    { "xp", AB::GameProtocol::CommandType::Xp },
    { "taunt", AB::GameProtocol::CommandType::Taunt },
    { "ponder", AB::GameProtocol::CommandType::Ponder },
    { "wave", AB::GameProtocol::CommandType::Wave },
    { "laugh", AB::GameProtocol::CommandType::Laugh },
    { "resign", AB::GameProtocol::CommandType::Resign },
    { "stuck", AB::GameProtocol::CommandType::Stuck },
    // Admin/GM
    { "pos", AB::GameProtocol::CommandType::Pos },
    { "id", AB::GameProtocol::CommandType::ServerId },
    { "entermap", AB::GameProtocol::CommandType::EnterMap },
    { "enterinstance", AB::GameProtocol::CommandType::EnterInstance },
    { "instances", AB::GameProtocol::CommandType::Instances },
    { "die", AB::GameProtocol::CommandType::Die },
    { "god", AB::GameProtocol::CommandType::GodMode },
    { "gotoplayer", AB::GameProtocol::CommandType::GotoPlayer },
    { "gminfo", AB::GameProtocol::CommandType::GMInfo },

    // Internal, handled by the client
    { "help", AB::GameProtocol::CommandType::Help },
    { "clear", AB::GameProtocol::CommandType::Clear },
    { "history", AB::GameProtocol::CommandType::History },
    { "ip", AB::GameProtocol::CommandType::Ip },
    { "prefpath", AB::GameProtocol::CommandType::PrefPath },
    { "quit", AB::GameProtocol::CommandType::Quit },
    { "time", AB::GameProtocol::CommandType::Time },
    { "cp", AB::GameProtocol::CommandType::ClientPrediction }
};

ChatWindow::ChatWindow(Context* context) :
    UIElement(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    XMLFile* chatFile = cache->GetResource<XMLFile>("UI/ChatWindow.xml");
    LoadChildXML(chatFile->GetRoot(), nullptr);
    SetName("ChatWindow");

    // Set self to same size as the window so align works
    Window* wnd = GetChildStaticCast<Window>("ChatWindow", true);
    wnd->SetPosition(0, 0);
    SetSize(wnd->GetSize());

    // Chat filter
    UIElement* filterContainer = wnd->GetChild("ChatFilter", false);
    {
        CheckBox* filterCheck = filterContainer->GetChildStaticCast<CheckBox>("ChatFilterGeneral", true);
        filterCheck->SetChecked(visibleGeneral_);
        filterCheck->SetVar("Channel", static_cast<unsigned>(AB::GameProtocol::ChatChannel::General));
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = filterContainer->GetChildStaticCast<CheckBox>("ChatFilterGuild", true);
        filterCheck->SetChecked(visibleGuild_);
        filterCheck->SetVar("Channel", static_cast<unsigned>(AB::GameProtocol::ChatChannel::Guild));
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = filterContainer->GetChildStaticCast<CheckBox>("ChatFilterParty", true);
        filterCheck->SetChecked(visibleParty_);
        filterCheck->SetVar("Channel", static_cast<unsigned>(AB::GameProtocol::ChatChannel::Party));
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = filterContainer->GetChildStaticCast<CheckBox>("ChatFilterTrade", true);
        filterCheck->SetChecked(visibleTrade_);
        filterCheck->SetVar("Channel", static_cast<unsigned>(AB::GameProtocol::ChatChannel::Trade));
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }
    {
        CheckBox* filterCheck = filterContainer->GetChildStaticCast<CheckBox>("ChatFilterWhisper", true);
        filterCheck->SetChecked(visibleWhisper_);
        filterCheck->SetVar("Channel", static_cast<unsigned>(AB::GameProtocol::ChatChannel::Whisper));
        SubscribeToEvent(filterCheck, E_TOGGLED, URHO3D_HANDLER(ChatWindow, HandleFilterClick));
    }

    UIElement* container = wnd->GetChild("ChatTextContainer", false);
    tabgroup_ = container->CreateChild<TabGroup>();
    tabgroup_->SetDefaultStyle(GetSubsystem<UI>()->GetRoot()->GetDefaultStyle());
    tabgroup_->SetAlignment(HA_CENTER, VA_BOTTOM);
    tabgroup_->SetColor(Color(0, 0, 0, 0));
    tabgroup_->SetStyleAuto();
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannel::General);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannel::Guild);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannel::Party);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannel::Trade);
    CreateChatTab(tabgroup_, AB::GameProtocol::ChatChannel::Whisper);

    tabgroup_->SetEnabled(true);
    SubscribeToEvent(E_TABSELECTED, URHO3D_HANDLER(ChatWindow, HandleTabSelected));

    chatLog_ = GetChildStaticCast<ListView>("ChatLog", true);

    SubscribeToEvent(Events::E_SCREENSHOTTAKEN, URHO3D_HANDLER(ChatWindow, HandleScreenshotTaken));
    SubscribeToEvent(Events::E_SERVERMESSAGE, URHO3D_HANDLER(ChatWindow, HandleServerMessage));
    SubscribeToEvent(Events::E_CHATMESSAGE, URHO3D_HANDLER(ChatWindow, HandleChatMessage));
    SubscribeToEvent(Events::E_SC_CHATGENERAL, URHO3D_HANDLER(ChatWindow, HandleShortcutChatGeneral));
    SubscribeToEvent(Events::E_SC_CHATGUILD, URHO3D_HANDLER(ChatWindow, HandleShortcutChatGuild));
    SubscribeToEvent(Events::E_SC_CHATPARTY, URHO3D_HANDLER(ChatWindow, HandleShortcutChatParty));
    SubscribeToEvent(Events::E_SC_CHATTRADE, URHO3D_HANDLER(ChatWindow, HandleShortcutChatTrade));
    SubscribeToEvent(Events::E_SC_CHATWHISPER, URHO3D_HANDLER(ChatWindow, HandleShortcutChatWhisper));
    SubscribeToEvent(Events::E_OBJECTPINGTARGET, URHO3D_HANDLER(ChatWindow, HandleTargetPinged));
    SubscribeToEvent(Events::E_PARTYRESIGNED, URHO3D_HANDLER(ChatWindow, HandlePartyResigned));
    SubscribeToEvent(Events::E_PARTYDEFEATED, URHO3D_HANDLER(ChatWindow, HandlePartyDefeated));
    SubscribeToEvent(Events::E_OBJECTITEMDROPPED, URHO3D_HANDLER(ChatWindow, HandleItemDropped));
    SubscribeToEvent(Events::E_OBJECTPROGRESS, URHO3D_HANDLER(ChatWindow, HandleObjectProgress));
    SubscribeToEvent(Events::E_WHISPERTO, URHO3D_HANDLER(ChatWindow, HandleWhisperTo));
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(ChatWindow, HandleKeyDown));

    SetAlignment(HA_LEFT, VA_BOTTOM);

    auto* options = GetSubsystem<Options>();
    historyRows_ = options->GetChatInputHistorySize();
    LoadHistory();
}

ChatWindow::~ChatWindow()
{
    SaveHistory();
    UnsubscribeFromAllEvents();
}

void ChatWindow::FocusEdit()
{
    LineEdit* edit = GetActiveLineEdit();
    if (edit && !edit->HasFocus())
        edit->SetFocus(true);
}

void ChatWindow::CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatChannel channel)
{
    static const IntVector2 tabSize(90, 20);
    static const IntVector2 tabBodySize(500, 20);

    Shortcuts* scs = GetSubsystem<Shortcuts>();
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    TabElement* tabElement = tabs->CreateTab(tabSize, tabBodySize);
    tabElement->tabText_->SetFont(cache->GetResource<Font>("Fonts/ClearSans-Regular.ttf"), 10);
    switch (channel)
    {
    case AB::GameProtocol::ChatChannel::General:
        tabElement->tabText_->SetText(scs->GetCaption(Events::E_SC_CHATGENERAL, "General", true));
        break;
    case AB::GameProtocol::ChatChannel::Party:
        tabElement->tabText_->SetText(scs->GetCaption(Events::E_SC_CHATPARTY, "Party", true));
        break;
    case AB::GameProtocol::ChatChannel::Guild:
        tabElement->tabText_->SetText(scs->GetCaption(Events::E_SC_CHATGUILD, "Guild", true));
        break;
    case AB::GameProtocol::ChatChannel::Trade:
        tabElement->tabText_->SetText(scs->GetCaption(Events::E_SC_CHATTRADE, "Trade", true));
        break;
    case AB::GameProtocol::ChatChannel::Whisper:
        tabElement->tabText_->SetText(scs->GetCaption(Events::E_SC_CHATWHISPER, "Whisper", true));
        tabIndexWhisper_ = static_cast<int>(tabs->GetTabCount() - 1);
        break;
    default:
        break;
    }

    UIElement* parent = tabElement->tabBody_;
    if (channel == AB::GameProtocol::ChatChannel::Whisper)
    {
        UIElement* container = parent->CreateChild<UIElement>();
        container->SetSize(parent->GetSize());
        LineEdit* nameEdit = container->CreateChild<LineEdit>();
        nameEdit->SetPosition(0, 0);
        nameEdit->SetName("WhisperChatNameEdit");
        nameEdit->SetAlignment(HA_LEFT, VA_CENTER);
        nameEdit->SetStyle("ChatLineEdit");
        nameEdit->SetSize(150, 20);
        SubscribeToEvent(nameEdit, E_FOCUSED, [](StringHash, VariantMap& eventData) {
            using namespace Focused;
            UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
            elem->SetOpacity(1.0f);
        });
        SubscribeToEvent(nameEdit, E_DEFOCUSED, [](StringHash, VariantMap& eventData) {
            using namespace Defocused;
            UIElement* elem = dynamic_cast<UIElement*>(eventData[P_ELEMENT].GetPtr());
            elem->SetOpacity(0.7f);
        });
        parent = container;
    }

    LineEdit* edit = parent->CreateChild<LineEdit>();
    edit->SetName("ChatLineEdit");
    edit->SetVar("Channel", static_cast<int>(channel));
    if (channel != AB::GameProtocol::ChatChannel::Whisper)
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
    if (index < 0)
        return nullptr;
    TabElement* elem = tabgroup_->GetTabElement(static_cast<unsigned>(index));
    if (!elem)
        return nullptr;
    LineEdit* edit = elem->tabBody_->GetChildStaticCast<LineEdit>("ChatLineEdit", true);
    return edit;
}

void ChatWindow::LoadHistory()
{
    auto* options = GetSubsystem<Options>();
    String filename = AddTrailingSlash(options->GetPrefPath()) + "history.txt";
    std::ifstream file(filename.CString());
    if (!file.is_open())
        return;
    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
            continue;
        history_.Push(ToUrhoString(line));
    }
    while (history_.Size() > historyRows_)
        history_.Erase(history_.Begin());
    historyPosition_ = history_.Size(); // Reset
}

void ChatWindow::SaveHistory()
{
    auto* options = GetSubsystem<Options>();
    String filename = AddTrailingSlash(options->GetPrefPath()) + "history.txt";
    std::fstream file(ToStdString(filename), std::fstream::out);
    for (const auto& line : history_)
    {
        file << line.CString() << std::endl;
    }
}

bool ChatWindow::MatchesFilter(const String& value)
{
    auto* chatFilter = GetSubsystem<ChatFilter>();
    return chatFilter->Matches(value);
}

void ChatWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<ChatWindow>();
}

void ChatWindow::HandleServerMessage(StringHash, VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    AB::GameProtocol::ServerMessageType type =
        static_cast<AB::GameProtocol::ServerMessageType>(eventData[P_MESSAGETYPE].GetInt());
    switch (type)
    {
    case AB::GameProtocol::ServerMessageType::Info:
        HandleServerMessageInfo(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Roll:
        HandleServerMessageRoll(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Age:
        HandleServerMessageAge(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Pos:
        HandleServerMessagePos(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Hp:
        HandleServerMessageHp(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Xp:
        HandleServerMessageXp(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Deaths:
        HandleServerMessageDeaths(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::PlayerNotOnline:
        HandleServerMessagePlayerNotOnline(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::PlayerGotMessage:
        HandleServerMessagePlayerGotMessage(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::NewMail:
        HandleServerMessageNewMail(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::MailSent:
        HandleServerMessageMailSent(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::MailNotSent:
        HandleServerMessageMailNotSent(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::MailboxFull:
        HandleServerMessageMailboxFull(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::MailDeleted:
        HandleServerMessageMailDeleted(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::ServerId:
        HandleServerMessageServerId(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::PlayerResigned:
        HandleServerMessagePlayerResigned(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::UnknownCommand:
        HandleServerMessageUnknownCommand(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::Instances:
        HandleServerMessageInstances(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::GMInfo:
        HandleServerMessageGMInfo(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::AdminMessage:
        HandleServerMessageAdminMessage(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::PlayerNotFound:
        HandleServerMessagePlayerNotFound(eventData);
        break;
    case AB::GameProtocol::ServerMessageType::PlayerQueued:
    case AB::GameProtocol::ServerMessageType::PlayerUnqueued:
        // TODO:
        break;
    case AB::GameProtocol::ServerMessageType::Unknown:
        break;
    }
}

void ChatWindow::HandleObjectProgress(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectProgress;
    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    AB::GameProtocol::ObjectProgressType type = static_cast<AB::GameProtocol::ObjectProgressType>(eventData[P_TYPE].GetUInt());
    switch (type)
    {
    case AB::GameProtocol::ObjectProgressType::GotSkillPoint:
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        Actor* actor = To<Actor>(lm->GetObject(objectId));
        if (actor)
        {
            static constexpr const char* TEMPLATE = "${name} got a skill point";
            const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [actor](const sa::templ::Token& token) -> std::string {
                switch (token.type)
                {
                case sa::templ::Token::Type::Variable:
                    if (token.value == "name")
                        return ToStdString(actor->name_);
                    ASSERT_FALSE();
                default:
                    return token.value;
                }
            });
            AddLine(ToUrhoString(t), "ChatLogServerInfoText");
        }
        break;
    }
    case AB::GameProtocol::ObjectProgressType::AttribPointsGain:
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        auto player = lm->GetPlayer();
        if (player && player->gameId_ == objectId)
        {
            static constexpr const char* TEMPLATE = "You got ${number} attribute points";
            const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&eventData](const sa::templ::Token& token) -> std::string
            {
                switch (token.type)
                {
                case sa::templ::Token::Type::Variable:
                    if (token.value == "number")
                        return std::to_string(eventData[P_VALUE].GetInt());
                    ASSERT_FALSE();
                default:
                    return token.value;
                }
            });
            AddLine(ToUrhoString(t), "ChatLogServerInfoText");
        }
        break;
    }
    case AB::GameProtocol::ObjectProgressType::LevelAdvance:
    {
        LevelManager* lm = GetSubsystem<LevelManager>();
        Actor* actor = To<Actor>(lm->GetObject(objectId));
        if (actor)
        {
            static constexpr const char* TEMPLATE = "${name} advanced to level ${level}";
            const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [actor, &eventData](const sa::templ::Token& token) -> std::string
            {
                switch (token.type)
                {
                case sa::templ::Token::Type::Variable:
                    if (token.value == "name")
                        return ToStdString(actor->name_);
                    if (token.value == "level")
                        return std::to_string(eventData[P_VALUE].GetInt());
                    ASSERT_FALSE();
                default:
                    return token.value;
                }
            });
            AddLine(ToUrhoString(t), "ChatLogServerInfoText");
        }
        break;
    }
    case AB::GameProtocol::ObjectProgressType::XPIncreased:
        // Do nothing
        break;
    }
}

void ChatWindow::HandleServerMessageUnknownCommand(VariantMap&)
{
    AddLine("Unknown command", "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageInfo(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    AddLine(message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageRoll(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    const String& sender = eventData[P_SENDER].GetString();
    unsigned p = message.Find(":");
    String res = message.Substring(0, p);
    String max = message.Substring(p + 1);

    static constexpr const char* TEMPLATE = "${name} rolls ${res} on a ${max} sided die.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "name")
                return ToStdString(sender);
            if (token.value == "res")
                return ToStdString(res);
            if (token.value == "max")
                return ToStdString(max);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogChatText");
}

void ChatWindow::HandleServerMessageAge(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    unsigned p = message.Find(":");
    String age = message.Substring(0, p);
    String playTime = message.Substring(p + 1);
    // Seconds
    uint32_t uAge = static_cast<uint32_t>(std::atoi(age.CString()));
    // Seconds
    uint32_t uPlayTime = static_cast<uint32_t>(std::atoi(playTime.CString()));
    const sa::time::time_span tAge = sa::time::get_time_span(uAge);

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("You have played this character for ");

    uint32_t hours = uPlayTime / 3600;
    if (hours > 0)
        uPlayTime -= hours * 3600;
    uint32_t minutes = uPlayTime / 60;
    if (minutes > 0)
        uPlayTime -= minutes * 60;
    if (hours > 0)
        parser.Append("${hours} hour(s) ", tokens);
    parser.Append("${minutes} minute(s) over the past ", tokens);
    if (tAge.months > 0)
        parser.Append("${months} month(s).", tokens);
    else
        parser.Append("${days} day(s).", tokens);

    const std::string t = tokens.ToString([&](const sa::templ::Token& token) -> std::string {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "hours")
                return std::to_string(hours);
            if (token.value == "minutes")
                return std::to_string(minutes);
            if (token.value == "months")
                return std::to_string(tAge.months);
            if (token.value == "days")
                return std::to_string(tAge.days);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageHp(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "Health ${currHp}/${maxHp}, Energy ${currE}/${maxE}";

    auto parts = message.Split('|');

    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "currHp")
            {
                if (parts.Size() < 1)
                    return "";
                unsigned p = parts[0].Find(":");
                String currHp = parts[0].Substring(0, p);
                return ToStdString(currHp);
            }
            if (token.value == "maxHp")
            {
                if (parts.Size() == 0)
                    return "";
                unsigned p = parts[0].Find(":");
                String maxHp = parts[0].Substring(p + 1);
                return ToStdString(maxHp);
            }
            if (token.value == "currE")
            {
                if (parts.Size() < 2)
                    return "";
                unsigned p = parts[1].Find(":");
                String currE = parts[1].Substring(0, p);
                return ToStdString(currE);
            }
            if (token.value == "maxE")
            {
                if (parts.Size() < 2)
                    return "";
                unsigned p = parts[1].Find(":");
                String maxE = parts[1].Substring(p + 1);
                return ToStdString(maxE);
            }
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageXp(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "You have ${xp} XP and ${sp} Skill points";

    auto parts = message.Split('|');
    if (parts.Size() == 2)
    {
        const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
        {
            switch (token.type)
            {
            case sa::templ::Token::Type::Variable:
                if (token.value == "xp")
                    return ToStdString(parts[0]);
                if (token.value == "sp")
                    return ToStdString(parts[1]);
                ASSERT_FALSE();
            default:
                return token.value;
            }
        });
        AddLine(ToUrhoString(t), "ChatLogServerInfoText");
    }
}

void ChatWindow::HandleServerMessageDeaths(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "You died ${deaths} times. You gained ${xp} XP since last death.";

    auto parts = message.Split('|');
    if (parts.Size() == 2)
    {
        const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
        {
            switch (token.type)
            {
            case sa::templ::Token::Type::Variable:
                if (token.value == "deaths")
                    return ToStdString(parts[0]);
                if (token.value == "xp")
                    return ToStdString(parts[1]);
                ASSERT_FALSE();
            default:
                return token.value;
            }
        });
        AddLine(ToUrhoString(t), "ChatLogServerInfoText");
    }
}

void ChatWindow::HandleServerMessagePos(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& message = eventData[P_DATA].GetString();
    AddLine(message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerNotOnline(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& name = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "Player ${name} is not online.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "name")
                return ToStdString(name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerGotMessage(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();
    const String& data = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "{${name}} ${data}";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "name")
                return ToStdString(name);
            if (token.value == "data")
                return ToStdString(data);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageNewMail(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& count = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "You got a new mail, total ${count} mail(s).";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "count")
                return ToStdString(count);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });

    AddLine(ToUrhoString(t), "ChatLogServerInfoText");

    VariantMap& eData = GetEventDataMap();
    using namespace Events::NewMail;
    eData[P_COUNT] = atoi(count.CString());
    SendEvent(Events::E_NEWMAIL, eData);
}

void ChatWindow::HandleServerMessageMailSent(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();

    static constexpr const char* TEMPLATE = "Mail to ${recipient} was sent.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "recipient")
                return ToStdString(name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailNotSent(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& name = eventData[P_SENDER].GetString();

    static constexpr const char* TEMPLATE = "Mail to ${recipient} was not sent. Please check the name, or the mail box is full.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "recipient")
                return ToStdString(name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailboxFull(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& count = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "Your mailbox is full! You have ${count} mails. Please delete some, so people are able to send you mails.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "count")
                return ToStdString(count);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageMailDeleted(VariantMap&)
{
    AddLine("The mail was deleted.", "ChatLogServerInfoText");
}

void ChatWindow::HandleChatMessage(StringHash, VariantMap& eventData)
{
    using namespace Events::ChatMessage;
    AB::GameProtocol::ChatChannel channel =
        static_cast<AB::GameProtocol::ChatChannel>(eventData[P_MESSAGETYPE].GetInt());
    const String& message = eventData[P_DATA].GetString();
    const String& sender = eventData[P_SENDER].GetString();
    uint32_t senderId = static_cast<uint32_t>(eventData[P_SENDERID].GetInt());
    if (!MatchesFilter(sender) && !MatchesFilter(message))
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
    if (GetSubsystem<Shortcuts>()->IsDisabled())
        return;

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
    using namespace Events::WhisperTo;
    VariantMap& e = GetEventDataMap();
    e[P_NAME] = name;
    SendEvent(Events::E_WHISPERTO, e);
}

void ChatWindow::HandleWhisperTo(StringHash, VariantMap& eventData)
{
    using namespace Events::WhisperTo;
    const String& name = eventData[P_NAME].GetString();
    if (!name.Empty())
    {
        LineEdit* nameEdit = GetChildStaticCast<LineEdit>("WhisperChatNameEdit", true);
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
    using namespace Events::ServerMessage;
    const String& id = eventData[P_DATA].GetString();
    AddLine(id, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerResigned(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& resigner = eventData[P_SENDER].GetString();

    static constexpr const char* TEMPLATE = "${name} has resigned";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "name")
                return ToStdString(resigner);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageInstances(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& instances = eventData[P_DATA].GetString();
    auto instVec = instances.Split(';');

    static constexpr const char* TEMPLATE = "${instance}: ${name} (${game})";
    sa::templ::Parser parser;
    const sa::templ::Tokens tokens = parser.Parse(TEMPLATE);

    for (auto& inst : instVec)
    {
        auto instData = inst.Split(',');
        if (instData.Size() != 3)
            continue;

        const std::string t = tokens.ToString([&](const sa::templ::Token& token) -> std::string
        {
            switch (token.type)
            {
            case sa::templ::Token::Type::Variable:
                if (token.value == "instance")
                    return ToStdString(instData[0]);
                if (token.value == "name")
                    return ToStdString(instData[1]);
                if (token.value == "game")
                    return ToStdString(instData[2]);
                ASSERT_FALSE();
            default:
                return token.value;
            }
        });
        AddLine(ToUrhoString(t), "ChatLogServerInfoText");
    }
}

void ChatWindow::HandleServerMessageGMInfo(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& sender = eventData[P_SENDER].GetString();
    const String& message = eventData[P_DATA].GetString();
    AddLine(sender, message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessageAdminMessage(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& sender = eventData[P_SENDER].GetString();
    const String& message = eventData[P_DATA].GetString();
    AddLine(sender, message, "ChatLogServerInfoText");
}

void ChatWindow::HandleServerMessagePlayerNotFound(VariantMap& eventData)
{
    using namespace Events::ServerMessage;
    const String& name = eventData[P_DATA].GetString();

    static constexpr const char* TEMPLATE = "A Player with name ${name} does not exist.";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "name")
                return ToStdString(name);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
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
    using namespace Events::PartyResigned;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();

    static constexpr const char* TEMPLATE = "Party ${id} has resigned";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "id")
                return std::to_string(partyId);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandlePartyDefeated(StringHash, VariantMap& eventData)
{
    using namespace Events::PartyDefeated;
    uint32_t partyId = eventData[P_PARTYID].GetUInt();

    static constexpr const char* TEMPLATE = "Party ${id} was defeated";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "id")
                return std::to_string(partyId);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

void ChatWindow::HandleTargetPinged(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectPingTarget;

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("I am");

    uint32_t objectId = eventData[P_OBJECTID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    AB::GameProtocol::ObjectCallType type = static_cast<AB::GameProtocol::ObjectCallType>(eventData[P_CALLTTYPE].GetUInt());
    int skillIndex = eventData[P_SKILLINDEX].GetInt();
    LevelManager* lm = GetSubsystem<LevelManager>();
    Actor* pinger = To<Actor>(lm->GetObject(objectId));
    Actor* target = To<Actor>(lm->GetObject(targetId));

    switch (type)
    {
    case AB::GameProtocol::ObjectCallType::Follow:
        if (!target)
            return;
        parser.Append(" following ${target}", tokens);
        break;
    case AB::GameProtocol::ObjectCallType::Using:
        if (!target)
            return;
        parser.Append(" using ${target}", tokens);
        break;
    case AB::GameProtocol::ObjectCallType::TalkingTo:
        if (!target)
            return;
        parser.Append(" talking to ${target}", tokens);
        break;
    case AB::GameProtocol::ObjectCallType::PickingUp:
        if (!target)
            return;
        parser.Append(" picking up ${target}", tokens);
        break;
    case AB::GameProtocol::ObjectCallType::Attack:
        if (!target)
            return;
        parser.Append(" attacking ${target}", tokens);
        break;
    case AB::GameProtocol::ObjectCallType::Target:
        if (!target)
            return;
        tokens = parser.Parse("${target} is my target");
        break;
    case AB::GameProtocol::ObjectCallType::UseSkill:
    {
        if (skillIndex <= 0)
            return;
        parser.Append(" using ${skill}", tokens);
        if (target)
            parser.Append(" on ${target}", tokens);
        break;
    }
    default:
        break;
    }

    const std::string t = tokens.ToString([&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "target")
                return ToStdString(target->name_);
            if (token.value == "skill")
            {
                ASSERT(skillIndex > 0);
                const AB::Entities::Skill* skill = GetSubsystem<SkillManager>()->GetSkillByIndex(pinger->skills_[static_cast<size_t>(skillIndex) - 1]);
                return skill->name;
            }
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddChatLine(objectId, pinger->name_, ToUrhoString(t), AB::GameProtocol::ChatChannel::Party);
    pinger->ShowSpeechBubble(ToUrhoString(t));
}

void ChatWindow::HandleItemDropped(StringHash, VariantMap& eventData)
{
    using namespace Events::ObjectItemDropped;
    uint32_t dropperId = eventData[P_OBJECTID].GetUInt();
    uint32_t targetId = eventData[P_TARGETID].GetUInt();
    uint32_t itemIndex = eventData[P_ITEMINDEX].GetUInt();
    uint32_t count = eventData[P_COUNT].GetUInt();

    LevelManager* lm = GetSubsystem<LevelManager>();
    Actor* dropper = To<Actor>(lm->GetObject(dropperId));
    Actor* target = To<Actor>(lm->GetObject(targetId));
    if (!target)
        return;

    ItemsCache* items = GetSubsystem<ItemsCache>();
    // Item may not be spawned yet
    Item* item = items->Get(itemIndex);

    sa::templ::Parser parser;
    sa::templ::Tokens tokens = parser.Parse("${dropper} dropped");
    if (count > 1)
        parser.Append(" ${count}", tokens);
    parser.Append(" ${item} for ${target}", tokens);

    const std::string t = tokens.ToString([&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "dropper")
                return ToStdString(dropper->name_);
            if (token.value == "count")
                return std::to_string(count);
            if (token.value == "item")
            {
                if (item)
                    return ToStdString(item->name_);
                return std::to_string(itemIndex);
            }
            if (token.value == "target")
                return ToStdString(target->name_);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
}

bool ChatWindow::ParseChatCommand(const String& text, AB::GameProtocol::ChatChannel defChannel)
{
    AB::GameProtocol::CommandType type = AB::GameProtocol::CommandType::Unknown;
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
        case AB::GameProtocol::ChatChannel::Guild:
            type = AB::GameProtocol::CommandType::ChatGuild;
            data = text;
            break;
        case AB::GameProtocol::ChatChannel::Party:
            type = AB::GameProtocol::CommandType::ChatParty;
            data = text;
            break;
        case AB::GameProtocol::ChatChannel::Trade:
            type = AB::GameProtocol::CommandType::ChatTrade;
            data = text;
            break;
        case AB::GameProtocol::ChatChannel::Whisper:
        {
            type = AB::GameProtocol::CommandType::ChatWhisper;
            LineEdit* nameEdit = GetChildStaticCast<LineEdit>("WhisperChatNameEdit", true);
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
            type = AB::GameProtocol::CommandType::ChatGeneral;
            data = text;
            break;
        }
    }

    switch (type)
    {
    case AB::GameProtocol::CommandType::Help:
        AddLine("Available commands:", "ChatLogServerInfoText");
        AddLine("  /a <message>: General chat", "ChatLogServerInfoText");
        AddLine("  /g <message>: Guild chat", "ChatLogServerInfoText");
        AddLine("  /party <message>: Party chat", "ChatLogServerInfoText");
        AddLine("  /trade <message>: Trade chat", "ChatLogServerInfoText");
        AddLine("  /w <name>, <message>: Whisper to <name> a <message>", "ChatLogServerInfoText");
        AddLine("  /roll <number>: Rolls a <number>-sided die (2-100 sides)", "ChatLogServerInfoText");
        AddLine("  /resign: Resign", "ChatLogServerInfoText");
        AddLine("  /age: Show Character age", "ChatLogServerInfoText");
        AddLine("  /deaths: Show how often you died", "ChatLogServerInfoText");
        AddLine("  /hp: Show health points and energy", "ChatLogServerInfoText");
        AddLine("  /stuck: Force server position", "ChatLogServerInfoText");
        AddLine("  /ip: Show server IP", "ChatLogServerInfoText");
        AddLine("  /prefpath: Show preferences path", "ChatLogServerInfoText");
        AddLine("  /help: Show this help", "ChatLogServerInfoText");
        AddLine("  /clear: Clear chat log", "ChatLogServerInfoText");
        AddLine("  /history: Show chat input history", "ChatLogServerInfoText");
        AddLine("  /time: Print Server and Client time", "ChatLogServerInfoText");
        AddLine("  /cp: Turn client prediction on/off", "ChatLogServerInfoText");
        AddLine("  /quit: Exit program", "ChatLogServerInfoText");
        break;
    case AB::GameProtocol::CommandType::Ip:
    {
        FwClient* client = GetSubsystem<FwClient>();
        uint32_t ip = client->GetIp();
        char buffer[20];
        sprintf(buffer, "%d.%d.%d.%d", ip >> 24, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        String sIp(buffer);
        AddLine(sIp, "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandType::PrefPath:
    {
        AddLine(Options::GetPrefPath(), "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandType::Clear:
    {
        chatLog_->RemoveAllItems();
        break;
    }
    case AB::GameProtocol::CommandType::History:
    {
        for (const auto& h : history_)
            AddLine(h, "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandType::Quit:
    {
        VariantMap& e = GetEventDataMap();
        SendEvent(Events::E_SC_EXITPROGRAM, e);
        break;
    }
    case AB::GameProtocol::CommandType::Time:
    {
        auto* client = GetSubsystem<FwClient>();
        const std::string value = sa::templ::Parser::Evaluate("Server: ${server}, Client: ${client}, Difference: ${diff}",
            [client](const sa::templ::Token& token) -> std::string
        {
            if (token.value == "server")
            {
                return sa::time::format_tick(client->GetServerTick());
            }
            if (token.value == "client")
            {
                return sa::time::format_tick(sa::time::tick());
            }
            if (token.value == "diff")
            {
                int64_t diff = client->GetClockDiff();
                if (abs(diff) < 1000)
                    return std::to_string(diff) + " ms";
                return std::to_string(diff / 1000) + " s";
            }
            ASSERT_FALSE();
        });
        AddLine(ToUrhoString(value), "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandType::ClientPrediction:
    {
        extern bool gNoClientPrediction;
        gNoClientPrediction = !gNoClientPrediction;
        String value = gNoClientPrediction ? "Client prediction is off" :
            "Client prediction is on";
        AddLine(value, "ChatLogServerInfoText");
        break;
    }
    case AB::GameProtocol::CommandType::Unknown:
        AddLine("Unknown command", "ChatLogServerInfoText");
        break;
    default:
    {
        if (data.Length() <= MAX_CHAT_MESSAGE)
        {
            FwClient* client = GetSubsystem<FwClient>();
            client->Command(type, data.Substring(0, MAX_CHAT_MESSAGE));
        }
        else
            AddLine("The message is too long", "ChatLogServerInfoText");
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
        AB::GameProtocol::ChatChannel channel = static_cast<AB::GameProtocol::ChatChannel>(elem->GetVar("Channel").GetUInt());
        switch (channel)
        {
        case AB::GameProtocol::ChatChannel::General:
            elem->SetVisible(visibleGeneral_);
            break;
        case AB::GameProtocol::ChatChannel::Guild:
            elem->SetVisible(visibleGuild_);
            break;
        case AB::GameProtocol::ChatChannel::Party:
            elem->SetVisible(visibleParty_);
            break;
        case AB::GameProtocol::ChatChannel::Trade:
            elem->SetVisible(visibleTrade_);
            break;
        case AB::GameProtocol::ChatChannel::Whisper:
            elem->SetVisible(visibleWhisper_);
            break;
        default:
            break;
        }
    }
}

void ChatWindow::HandleFilterClick(StringHash, VariantMap& eventData)
{
    using namespace Toggled;
    CheckBox* sender = dynamic_cast<CheckBox*>(eventData[P_ELEMENT].GetPtr());
    bool state = eventData[P_STATE].GetBool();
    AB::GameProtocol::ChatChannel channel = static_cast<AB::GameProtocol::ChatChannel>(sender->GetVar("Channel").GetUInt());
    switch (channel)
    {
    case AB::GameProtocol::ChatChannel::General:
        visibleGeneral_ = state;
        break;
    case AB::GameProtocol::ChatChannel::Guild:
        visibleGuild_ = state;
        break;
    case AB::GameProtocol::ChatChannel::Party:
        visibleParty_ = state;
        break;
    case AB::GameProtocol::ChatChannel::Trade:
        visibleTrade_ = state;
        break;
    case AB::GameProtocol::ChatChannel::Whisper:
        visibleWhisper_ = state;
        break;
    default:
        return;
    }
    UpdateVisibleItems();
}

void ChatWindow::HandleScreenshotTaken(StringHash, VariantMap& eventData)
{
    using namespace Events::ScreenshotTaken;
    const String& file = eventData[P_FILENAME].GetString();

    static constexpr const char* TEMPLATE = "Screenshot saved to ${file}";
    const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
    {
        switch (token.type)
        {
        case sa::templ::Token::Type::Variable:
            if (token.value == "file")
                return ToStdString(file);
            ASSERT_FALSE();
        default:
            return token.value;
        }
    });
    AddLine(ToUrhoString(t), "ChatLogServerInfoText");
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
        AB::GameProtocol::ChatChannel channel =
            static_cast<AB::GameProtocol::ChatChannel>(sender->GetVar("Channel").GetInt());
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
    AB::GameProtocol::ChatChannel channel /* = AB::GameProtocol::ChatChannel::Unknown */)
{
    Text* nameText = chatLog_->CreateChild<Text>();
    nameText->SetVar("ID", id);
    nameText->SetVar("Name", name);
    nameText->SetVar("Channel", static_cast<unsigned>(channel));
    if (channel != AB::GameProtocol::ChatChannel::Whisper)
        nameText->SetText(name + ":");
    else
        nameText->SetText("{" + name + "}");
    switch (channel)
    {
    case AB::GameProtocol::ChatChannel::General:
        nameText->SetVisible(visibleGeneral_);
        break;
    case AB::GameProtocol::ChatChannel::Guild:
        nameText->SetVisible(visibleGuild_);
        break;
    case AB::GameProtocol::ChatChannel::Party:
        nameText->SetVisible(visibleParty_);
        break;
    case AB::GameProtocol::ChatChannel::Trade:
        nameText->SetVisible(visibleTrade_);
        break;
    case AB::GameProtocol::ChatChannel::Whisper:
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

void ChatWindow::AddChatLine(uint32_t senderId, const String& name, const String& text, AB::GameProtocol::ChatChannel channel)
{
    String style;
    String textStyle = "ChatLogChatText";
    switch (channel)
    {
    case AB::GameProtocol::ChatChannel::General:
        style = "ChatLogGeneralChatText";
        break;
    case AB::GameProtocol::ChatChannel::Guild:
        style = "ChatLogGuildChatText";
        break;
    case AB::GameProtocol::ChatChannel::Party:
        style = "ChatLogPartyChatText";
        break;
    case AB::GameProtocol::ChatChannel::Whisper:
        style = "ChatLogWhisperChatText";
        break;
    case AB::GameProtocol::ChatChannel::Trade:
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
    static bool firstStart = true;
    if (firstStart && player)
    {
        static constexpr const char* TEMPLATE = "Hello ${name}, type /help for available commands.";
        const std::string t = sa::templ::Parser::Evaluate(TEMPLATE, [&](const sa::templ::Token& token) -> std::string
        {
            switch (token.type)
            {
            case sa::templ::Token::Type::Variable:
                if (token.value == "name")
                    return ToStdString(player->name_);
                ASSERT_FALSE();
            default:
                return token.value;
            }
        });
        AddLine(ToUrhoString(t), "ChatLogServerInfoText");
        firstStart = false;
    }
}

void ChatWindow::SetHistorySize(unsigned value)
{
    if (historyRows_ == value)
        return;

    historyRows_ = value;
    while (history_.Size() > historyRows_)
        history_.Erase(history_.Begin());
}
