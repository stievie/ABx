#include "stdafx.h"
#include "MailWindow.h"

void MailWindow::RegisterObject(Context* context)
{
    context->RegisterFactory<MailWindow>();
}

MailWindow::MailWindow(Context* context) :
    Object(context),
    visible_(false)
{
    SubscribeToEvents();
}

void MailWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{

}

void MailWindow::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MailWindow, HandleUpdate));
}
