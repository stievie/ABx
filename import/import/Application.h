#pragma once

class Application
{
private:
    enum Action
    {
        Unknown,
        CreateHull,
        CreateHeightMap
    };
    Action action_ = Unknown;
    void ParseCommandLine();
    std::vector<std::string> files_;
public:
    Application() = default;
    ~Application() = default;

    bool Initialize(int argc, char** argv);
    void Run();

    std::vector<std::string> arguments_;
};

