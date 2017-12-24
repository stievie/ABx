#pragma once

class GameMenu : public UIElement
{
    URHO3D_OBJECT(GameMenu, UIElement);
private:
    void CreateMenuBar();
    Menu* CreateMenu(const String& name);
public:
    static void RegisterObject(Context* context);

    GameMenu(Context* context);
    ~GameMenu();
};

