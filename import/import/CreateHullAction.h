#pragma once

#include <assimp/vector3.h>

class CreateHullAction
{
private:
    std::string file_;
    unsigned vertexCount_;
    unsigned indexCount_;
    std::vector<aiVector3D> vertexData_;
    std::vector<unsigned> indexData_;
    void BuildHull(const std::vector<aiVector3D>& vertices);
    void Save();
public:
    CreateHullAction(const std::string& file) :
        file_(file)
    {}
    ~CreateHullAction() = default;

    void Execute();
};

