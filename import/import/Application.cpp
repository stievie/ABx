#include "stdafx.h"
#include "Application.h"
#include "CreateHullAction.h"
#include "CreateHeightMapAction.h"
#include "CreateModelAction.h"

bool Application::ParseCommandLine()
{
    for (int i = 1; i != arguments_.size(); i++)
    {
        const std::string& a = arguments_[i];
        if (a.compare("-hull") == 0)
        {
            action_ = CreateHull;
        }
        else if (a.compare("-hm") == 0)
        {
            action_ = CreateHeightMap;
        }
        else if (a.compare("-model") == 0)
        {
            action_ = CreateModel;
        }
        else if (a.compare("-h") == 0 || a.compare("-?") == 0)
        {
            return false;
        }
        else
            files_.push_back(a);
    }
    return action_ != Unknown;
}

void Application::ShowHelp()
{
    std::cout << "import -<action> [<options>] <file1>[...<fileN>]" << std::endl;
    std::cout << "action:" << std::endl;
    std::cout << "  h, ?: Show help" << std::endl;
    std::cout << "  hull: Create hull shape from model" << std::endl;
    std::cout << "  hm: Create height (for terrain) map from image" << std::endl;
    std::cout << "  model: Create model" << std::endl;
    std::cout << "  scene: Import scene" << std::endl;
}

bool Application::Initialize(int argc, char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        arguments_.push_back(std::string(argv[i]));
    }
    return true;
}

void Application::Run()
{
    if (!ParseCommandLine())
    {
        ShowHelp();
        return;
    }

    switch (action_)
    {
    case CreateHull:
        for (const auto& file : files_)
        {
            CreateHullAction action(file);
            action.Execute();
        }
        break;
    case CreateHeightMap:
        for (const auto& file : files_)
        {
            CreateHeightMapAction action(file);
            action.Execute();
        }
        break;
    case CreateModel:
        for (const auto& file : files_)
        {
            CreateModelAction action(file);
            action.Execute();
        }
        break;

    }
}
