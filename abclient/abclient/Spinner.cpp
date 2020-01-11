#include "stdafx.h"
#include "Spinner.h"

static const float DEFAULT_REPEAT_DELAY = 0.4f;
static const float DEFAULT_REPEAT_RATE = 20.0f;

void Spinner::RegisterObject(Context* context)
{
    context->RegisterFactory<Spinner>();
}

Spinner::Spinner(Context* context) :
    BorderImage(context)
{
    SetEditable(false);
    SetFocusMode(FM_FOCUSABLE);
    SetLayoutMode(LM_HORIZONTAL);

    SetLayoutMode(LM_VERTICAL);

    auto* buttonIncrease = CreateChild<Button>("Increase");
    buttonIncrease->SetInternal(true);
    buttonIncrease->SetRepeat(DEFAULT_REPEAT_DELAY, DEFAULT_REPEAT_RATE);
    buttonIncrease->SetFocusMode(FM_NOTFOCUSABLE);
    SubscribeToEvent(buttonIncrease, E_RELEASED, URHO3D_HANDLER(Spinner, HandleIncreaseClicked));

    auto* buttonDecrease = CreateChild<Button>("Decrease");
    buttonDecrease->SetInternal(true);
    buttonDecrease->SetRepeat(DEFAULT_REPEAT_DELAY, DEFAULT_REPEAT_RATE);
    buttonDecrease->SetFocusMode(FM_NOTFOCUSABLE);
    SubscribeToEvent(buttonDecrease, E_RELEASED, URHO3D_HANDLER(Spinner, HandleDecreaseClicked));

    SubscribeToEvent(E_MOUSEWHEEL, URHO3D_HANDLER(Spinner, HandleMouseWheel));

    UpdateLayout();
}

Spinner::~Spinner()
{
    UnsubscribeFromAllEvents();
}

void Spinner::SetMin(int value)
{
    if (min_ != value)
    {
        min_ = value;
        Validate();
    }
}

void Spinner::SetMax(int value)
{
    if (max_ != value)
    {
        max_ = value;
        Validate();
    }
}

void Spinner::SetValue(int value)
{
    if (value != value_)
    {
        value_ = value;
        Validate();
    }
}

void Spinner::Increase()
{
    if (value_ < max_)
    {
        value_ += step_;
        Validate();
    }
}

void Spinner::Decrease()
{
    if (value_ > min_)
    {
        value_ -= step_;
        Validate();
    }
}

bool Spinner::HaveFocus() const
{
    UI* ui = GetSubsystem<UI>();
    auto* f = ui->GetFocusElement();
    if (f == this)
        return true;
    if (auto e = edit_.Lock())
    {
        if (e.Get() == f)
            return true;
    }
    return false;
}

void Spinner::HandleMouseWheel(StringHash, VariantMap& eventData)
{
    if (!HaveFocus())
        return;

    using namespace MouseWheel;
    int v = eventData[P_WHEEL].GetInt();
    if (v > 0)
        Increase();
    else if (v < 0)
        Decrease();
}

void Spinner::HandleIncreaseClicked(StringHash, VariantMap&)
{
    Increase();
}

void Spinner::HandleDecreaseClicked(StringHash, VariantMap&)
{
    Decrease();
}

void Spinner::SetEdit(SharedPtr<LineEdit> value)
{
    edit_ = value;
    Validate();
}

void Spinner::Validate()
{
    value_ = Clamp(value_, min_, max_);
    if (auto e = edit_.Lock())
        e->SetText(String(value_));
}
