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
    windowPos_ = nk_vec2(50, 50);
    windowSize_ = nk_vec2(220, 220);
}

void MailWindow::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    if (!visible_)
        return;

    NuklearUI* nuklear = GetSubsystem<NuklearUI>();

    if (nk_begin(nuklear->GetNkContext(), "Mail",
        nk_rect(windowPos_.x, windowPos_.y, windowSize_.x, windowSize_.y),
        NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE | NK_WINDOW_CLOSABLE))
    {
        nk_layout_row_begin(nuklear->GetNkContext(), NK_DYNAMIC, 100, 1);
        {
            nk_layout_row_push(nuklear->GetNkContext(), 100);
            if (nk_button_label(nuklear->GetNkContext(), "Write"))
            {
                /* event handling */
            }


            nk_layout_row_push(nuklear->GetNkContext(), 100);
            // must be keep the fllow code between nk_begin... and nk_end...
            nk_flags event = nk_edit_string_zero_terminated(nuklear->GetNkContext(),
                NK_EDIT_BOX | NK_EDIT_AUTO_SELECT | NK_EDIT_MULTILINE, //fcous will auto select all text (NK_EDIT_BOX not sure)
                buffer_, sizeof(buffer_), nk_filter_ascii);//nk_filter_ascii Text Edit accepts text types.
        }
        nk_layout_row_end(nuklear->GetNkContext());

    }
    nk_end(nuklear->GetNkContext());

    visible_ = !nk_window_is_closed(nuklear->GetNkContext(), "Mail");
}

void MailWindow::SubscribeToEvents()
{
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MailWindow, HandleUpdate));
}
