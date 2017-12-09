#pragma once

#pragma warning( push )
#pragma warning( disable : 4100 4305)
#include <Urho3D/Urho3DAll.h>
#pragma warning( pop )

class ChatWindow : public UIElement
{
    URHO3D_OBJECT(ChatWindow, UIElement);
private:
    ListView* chatLog_;
    LineEdit* chatEdit_;
    SharedPtr<BorderImage> background_;
public:
    static void RegisterObject(Context* context);

    void AddLine(const String& text);

    ChatWindow(Context* context);
    ~ChatWindow();
};

