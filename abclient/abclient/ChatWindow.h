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
    static const HashMap<String, AB::GameProtocol::CommandTypes> CHAT_COMMANDS;
    SharedPtr<BorderImage> background_;
    bool firstStart_{ false };
    Vector<String> history_;
    /// Command auto complete current position.
    unsigned historyRows_{ 20 };
    /// Store the original line which is being auto-completed
    unsigned historyPosition_{ 0 };
    /// Current row being edited.
    String currentRow_;
    bool visibleGeneral_{ true };
    bool visibleGuild_{ true };
    bool visibleParty_{ true };
    bool visibleTrade_{ true };
    bool visibleWhisper_{ true };
    int tabIndexWhisper_{ -1 };
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
    bool ParseChatCommand(const String& text, AB::GameProtocol::ChatMessageChannel defChannel);
    void CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatMessageChannel channel);
    LineEdit* GetActiveLineEdit();
    LineEdit* GetLineEdit(int index);
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text, const String& style);
    void AddLine(const String& name, const String& text, const String& style);
    void AddLine(uint32_t id, const String& name, const String& text,
        const String& style, const String& style2 = String::EMPTY,
        AB::GameProtocol::ChatMessageChannel channel = AB::GameProtocol::ChatChannelUnknown);

    void AddChatLine(uint32_t senderId, const String& name, const String& text,
        AB::GameProtocol::ChatMessageChannel channel);
    void SayHello(Player* player);

    ChatWindow(Context* context);
    ~ChatWindow() override
    {
        UnsubscribeFromAllEvents();
    }

    void FocusEdit();
    SharedPtr<ListView> chatLog_;
private:
    SharedPtr<TabGroup> tabgroup_;
};

