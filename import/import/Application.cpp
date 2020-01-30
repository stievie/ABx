/**
 * Copyright 2020 Stefan Ascher
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "stdafx.h"
#include "Application.h"
#include "CreateHullAction.h"
#include "CreateHeightMapAction.h"
#include "CreateModelAction.h"
#include "CreateSceneAction.h"

bool Application::ParseCommandLine()
{
    for (size_t i = 1; i != arguments_.size(); i++)
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
        else if (a.compare("-scene") == 0)
        {
            action_ = CreateScene;
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
    case CreateScene:
        for (const auto& file : files_)
        {
            CreateSceneAction action(file);
            action.Execute();
        }
        break;
    default:
        return;
    }
}
