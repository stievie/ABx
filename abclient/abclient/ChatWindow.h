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

#pragma once

#include <AB/ProtocolCodes.h>
#include "TabGroup.h"
#include <Urho3DAll.h>

class Player;

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement)
private:
    static constexpr int MAX_LINES = 100;
    static const HashMap<String, AB::GameProtocol::CommandType> CHAT_COMMANDS;
    SharedPtr<BorderImage> background_;
    bool firstStart_{ false };
    Vector<String> history_;
    Vector<String> filterPatterns_;
    /// Command auto complete current position.
    unsigned historyRows_{ 20 };
    /// Store the original line which is being auto-completed
    unsigned historyPosition_{ 0 };
    /// Current row being edited.
    String currentRow_;
    int tabIndexWhisper_{ -1 };
    bool visibleGeneral_{ true };
    bool visibleGuild_{ true };
    bool visibleParty_{ true };
    bool visibleTrade_{ true };
    bool visibleWhisper_{ true };
    void TrimLines();
    void UpdateVisibleItems();
    void HandleFilterClick(StringHash eventType, VariantMap& eventData);
    void HandleScreenshotTaken(StringHash eventType, VariantMap& eventData);
    void HandleEditFocused(StringHash eventType, VariantMap& eventData);
    void HandleEditDefocused(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleEditKey(StringHash eventType, VariantMap& eventData);
    void HandleServerMessage(StringHash eventType, VariantMap& eventData);
    void HandleObjectProgress(StringHash eventType, VariantMap& eventData);
    void HandleServerMessageUnknownCommand(VariantMap&);
    void HandleServerMessageInfo(VariantMap& eventData);
    void HandleServerMessageRoll(VariantMap& eventData);
    void HandleServerMessageAge(VariantMap& eventData);
    void HandleServerMessageHp(VariantMap& eventData);
    void HandleServerMessageXp(VariantMap& eventData);
    void HandleServerMessagePos(VariantMap& eventData);
    void HandleServerMessagePlayerNotOnline(VariantMap& eventData);
    void HandleServerMessagePlayerGotMessage(VariantMap& eventData);
    void HandleServerMessageNewMail(VariantMap& eventData);
    void HandleServerMessageMailSent(VariantMap& eventData);
    void HandleServerMessageMailNotSent(VariantMap& eventData);
    void HandleServerMessageMailboxFull(VariantMap& eventData);
    void HandleServerMessageMailDeleted(VariantMap& eventData);
    void HandleServerMessageServerId(VariantMap& eventData);
    void HandleServerMessagePlayerResigned(VariantMap& eventData);
    void HandleServerMessageInstances(VariantMap& eventData);
    void HandleServerMessageGMInfo(VariantMap& eventData);
    void HandleServerMessagePlayerNotFound(VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void HandleWhisperTo(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatGeneral(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatGuild(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatParty(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatTrade(StringHash eventType, VariantMap& eventData);
    void HandleShortcutChatWhisper(StringHash eventType, VariantMap& eventData);
    void HandlePartyResigned(StringHash eventType, VariantMap& eventData);
    void HandlePartyDefeated(StringHash eventType, VariantMap& eventData);
    void HandleTargetPinged(StringHash eventType, VariantMap& eventData);
    void HandleItemDropped(StringHash eventType, VariantMap& eventData);
    bool ParseChatCommand(const String& text, AB::GameProtocol::ChatChannel defChannel);
    void CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatChannel channel);
    LineEdit* GetActiveLineEdit();
    LineEdit* GetLineEdit(int index);
    void LoadHistory();
    void SaveHistory();
    void LoadFilters();
    bool MatchesFilter(const String& value);
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text, const String& style);
    void AddLine(const String& name, const String& text, const String& style);
    void AddLine(uint32_t id, const String& name, const String& text,
        const String& style, const String& style2 = String::EMPTY,
        AB::GameProtocol::ChatChannel channel = AB::GameProtocol::ChatChannel::Unknown);

    void AddChatLine(uint32_t senderId, const String& name, const String& text,
        AB::GameProtocol::ChatChannel channel);
    void SayHello(Player* player);
    void SetHistorySize(unsigned value);

    ChatWindow(Context* context);
    ~ChatWindow() override;

    void FocusEdit();
    SharedPtr<ListView> chatLog_;
private:
    SharedPtr<TabGroup> tabgroup_;
};

