#pragma once

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement);
private:
    SharedPtr<BorderImage> background_;
    void HandleTextChanged(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleChatEditKey(StringHash eventType, VariantMap& eventData);
    void HandleHoverBegin(StringHash eventType, VariantMap& eventData);
    void HandleHoverEnd(StringHash eventType, VariantMap& eventData);
    void HandleServerMessage(StringHash eventType, VariantMap& eventData);
    void ParseChatCommand(const String& text);
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text, const String& style);

    ChatWindow(Context* context);
    ~ChatWindow();

    SharedPtr<ListView> chatLog_;
    SharedPtr<LineEdit> chatEdit_;
};

