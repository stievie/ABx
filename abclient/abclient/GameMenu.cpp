#include "stdafx.h"
#include "GameMenu.h"

void GameMenu::RegisterObject(Context* context)
{
    context->RegisterFactory<GameMenu>();
}

GameMenu::GameMenu(Context* context) :
    UIElement(context)
{
    CreateMenuBar();
}

GameMenu::~GameMenu()
{
    UnsubscribeFromAllEvents();
}

void GameMenu::CreateMenuBar()
{
    BorderImage* menuBar = CreateChild<BorderImage>("MenuBar");
    menuBar->SetEnabled(true);
    menuBar->SetLayout(LM_HORIZONTAL);

    Menu* menu = CreateMenu("File");
}

Menu* GameMenu::CreateMenu(const String& name)
{
    return nullptr;
}
