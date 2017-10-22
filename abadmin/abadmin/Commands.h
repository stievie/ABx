#pragma once

#include <functional>
#include <string>
#include <vector>
#include <map>

typedef int(*CommandFunc)(const std::vector<std::string>&);

class Command
{
private:
    CommandFunc function_;
public:
    Command() :
        function_(nullptr),
        help_("")
    {}
    Command(CommandFunc f, const std::string& help) :
        function_(f),
        help_(help)
    {}
    int Execute(const std::vector<std::string>& params)
    {
        if (function_)
            return function_(params);
        return -1;
    }
    std::string help_;
};

class Commands
{
public:
    Commands() = default;
    ~Commands() {}
    void Initialize();
    int Execute(const std::string& line);

    std::map<std::string, Command> commands_;

    static Commands Instance;
};

