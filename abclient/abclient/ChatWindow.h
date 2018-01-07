#pragma once

#include <AB/ProtocolCodes.h>
#include "TabGroup.h"

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement);
private:
    SharedPtr<BorderImage> background_;
    void HandleEditFocused(StringHash eventType, VariantMap& eventData);
    void HandleEditDefocused(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleServerMessage(StringHash eventType, VariantMap& eventData);
    void HandleChatMessage(StringHash eventType, VariantMap& eventData);
    void HandleTabSelected(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandleNameClicked(StringHash eventType, VariantMap& eventData);
    void ParseChatCommand(const String& text, AB::GameProtocol::ChatMessageChannel defChannel);
    void CreateChatTab(TabGroup* tabs, AB::GameProtocol::ChatMessageChannel channel);
    LineEdit* GetActiveLineEdit();
    LineEdit* GetLineEdit(int index);
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text, const String& style);
    void AddLine(uint32_t id, const String& name, const String& text,
        const String& style, const String& style2 = String::EMPTY, bool isWhisper = false);

    void AddChatLine(uint32_t senderId, const String& name, const String& text,
        AB::GameProtocol::ChatMessageChannel channel);

    ChatWindow(Context* context);
    ~ChatWindow()
    {
        UnsubscribeFromAllEvents();
    }

    void FocusEdit();
    SharedPtr<ListView> chatLog_;
private:
    SharedPtr<TabGroup> tabgroup_;
};

