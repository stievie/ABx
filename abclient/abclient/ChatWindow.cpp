#include "stdafx.h"
#include "ChatWindow.h"


ChatWindow::ChatWindow(Context* context) :
    UIElement(context)
{
    ResourceCache* cache = GetSubsystem<ResourceCache>();
    XMLFile* style = cache->GetResource<XMLFile>("UI/FwDefaultStyle.xml");
    SetDefaultStyle(style);
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
    UIElement* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetVoidPtr());
}

void ChatWindow::HandleHoverEnd(StringHash eventType, VariantMap& eventData)
{
    using namespace HoverEnd;
    UIElement* elem = static_cast<UIElement*>(eventData[P_ELEMENT].GetVoidPtr());
}

void ChatWindow::HandleTextChanged(StringHash eventType, VariantMap& eventData)
{

}

void ChatWindow::HandleTextFinished(StringHash eventType, VariantMap& eventData)
{
    using namespace TextFinished;

    String line = chatEdit_->GetText();
    if (!line.Empty())
    {
    }
}

void ChatWindow::HandleChatEditKey(StringHash eventType, VariantMap& eventData)
{
    using namespace UnhandledKey;
/*    switch (eventData[P_KEY].GetInt())
    {
    } */
}

void ChatWindow::AddLine(const String& text)
{
    Text* txt = new Text(context_);
    txt->SetText(text);
    txt->SetStyle("ChatLogServerInfoText");
    chatLog_->AddItem(txt);
    chatLog_->EnsureItemVisibility(txt);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}
