#pragma once

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement);
private:
    SharedPtr<ListView> chatLog_;
    SharedPtr<LineEdit> chatEdit_;
    SharedPtr<BorderImage> background_;
    void HandleTextChanged(StringHash eventType, VariantMap& eventData);
    void HandleTextFinished(StringHash eventType, VariantMap& eventData);
    void HandleChatEditKey(StringHash eventType, VariantMap& eventData);
    void HandleHoverBegin(StringHash eventType, VariantMap& eventData);
    void HandleHoverEnd(StringHash eventType, VariantMap& eventData);
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text);

    ChatWindow(Context* context);
    ~ChatWindow();
};

