#pragma once

class CreateSceneAction
{
private:
    std::string file_;
public:
    CreateSceneAction(const std::string& file) :
        file_(file)
    { }
    ~CreateSceneAction() = default;
    void Execute();
};

