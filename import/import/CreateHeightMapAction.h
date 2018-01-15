#pragma once
class CreateHeightMapAction
{
private:
    std::string file_;
public:
    CreateHeightMapAction(const std::string& file) :
        file_(file)
    {}
    ~CreateHeightMapAction() = default;
    void Execute();
};

