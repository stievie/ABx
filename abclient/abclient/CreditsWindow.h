#pragma once

class CreditsWindow : public Window
{
    URHO3D_OBJECT(CreditsWindow, Window);
private:
    Vector<SharedPtr<Text>> credits_;
    SharedPtr<UIElement> creditsBase_;
    int totalCreditsHeight_;
    int creditLengthInSeconds_;
    Timer timer_;
    void CreateUI();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void CreateSingleLine(const String& content, int fontSize);
    void AddCredits();
public:
    static void RegisterObject(Context* context);
    static String NAME;
    CreditsWindow(Context* context);
    ~CreditsWindow();
    void Close();
};

