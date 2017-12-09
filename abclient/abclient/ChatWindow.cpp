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

    chatLog_ = dynamic_cast<ListView*>(GetChild("ChatLog", true));
    chatEdit_ = dynamic_cast<LineEdit*>(GetChild("ChatEdit", true));
}

ChatWindow::~ChatWindow()
{
}

void ChatWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<ChatWindow>();
}

void ChatWindow::AddLine(const String& text)
{
    Text* txt = new Text(context_);
    txt->SetText(text);
    chatLog_->AddItem(txt);
    chatLog_->EnsureItemVisibility(txt);
    chatLog_->EnableLayoutUpdate();
    chatLog_->UpdateLayout();
}
