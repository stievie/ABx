#include "stdafx.h"
#include "Application.h"
#include "CreateHullAction.h"
#include "CreateHeightMapAction.h"

void Application::ParseCommandLine()
{
    for (int i = 0; i != arguments_.size(); i++)
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
        else
            files_.push_back(a);
    }
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
    ParseCommandLine();
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
    }
}
