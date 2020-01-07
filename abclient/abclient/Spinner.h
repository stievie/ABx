#pragma once

#include <Urho3DAll.h>

class Spinner : public BorderImage
{
    URHO3D_OBJECT(Spinner, BorderImage)
private:
    int min_{ 0 };
    int max_{ 100 };
    int value_{ 0 };
    WeakPtr<LineEdit> edit_;
    void HandleMouseWheel(StringHash eventType, VariantMap& eventData);
    void HandleIncreaseClicked(StringHash eventType, VariantMap& eventData);
    void HandleDecreaseClicked(StringHash eventType, VariantMap& eventData);
    void Validate();
public:
    static void RegisterObject(Context* context);

    Spinner(Context* context);
    ~Spinner() override;

    int GetMin() const { return min_; }
    void SetMin(int value);
    int GetMax() const { return max_; }
    void SetMax(int value);
    int GetValue() const { return value_; }
    void SetValue(int value);
    void Increase();
    void Decrease();
    void SetEdit(SharedPtr<LineEdit> value) { edit_ = value; }
};

