#pragma once

static const StringHash WINDOW_OPTIONS("OptionsWindow");
static const StringHash WINDOW_CHAT("CatWindow");
static const StringHash WINDOW_MAIL("MailWindow");
static const StringHash WINDOW_PARTY("PartyWindow");
static const StringHash WINDOW_PINGDOT("PingDot");
static const StringHash WINDOW_TARGET("TargetWindow");
static const StringHash WINDOW_NEWMAIL("NewMailWindow");
static const StringHash WINDOW_MISSIONMAP("MissionMapWindow");

class WindowManager : public Object
{
    URHO3D_OBJECT(WindowManager, Object);
private:
    HashMap<StringHash, SharedPtr<UIElement>> windows_;
public:
    WindowManager(Context* context);
    ~WindowManager();

    const HashMap<StringHash, SharedPtr<UIElement>>& GetWindows() const
    {
        return windows_;
    }
    bool IsLoaded(const StringHash& hash) const
    {
        return windows_.Contains(hash);
    }

    SharedPtr<UIElement> GetWindow(const StringHash& hash, bool addToUi = false);
    void SaveWindows();
};

