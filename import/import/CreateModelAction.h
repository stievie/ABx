#pragma once

#include <assimp/vector3.h>

class CreateModelAction
{
private:
    std::string file_;
    uint32_t vertexCount_;
    uint32_t indexCount_;
    std::vector<aiVector3D> vertexData_;
    std::vector<unsigned> indexData_;
    void Save();
public:
    CreateModelAction(const std::string& file) :
        vertexCount_(0),
        indexCount_(0),
        file_(file)
    { }
    ~CreateModelAction() = default;
    void Execute();
};

