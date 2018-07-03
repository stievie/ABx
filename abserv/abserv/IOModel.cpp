#include "stdafx.h"
#include "IOModel.h"
#include "Profiler.h"
#include "Shape.h"

namespace IO {

bool IOModel::Import(Game::Model* asset, const std::string& name)
{
    std::fstream input(name, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    AB_PROFILE;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'M' || sig[1] != 'O' || sig[2] != 'D' || sig[3] != 'L')
        return false;

    asset->shape_ = std::make_unique<Math::Shape>();

    input.read(reinterpret_cast<char*>(&asset->shape_->vertexCount_), sizeof(asset->shape_->vertexCount_));
    asset->shape_->vertexData_.resize(asset->shape_->vertexCount_);
    input.read((char*)asset->shape_->vertexData_.data(), sizeof(float) * 3 * asset->shape_->vertexCount_);

    input.read(reinterpret_cast<char*>(&asset->shape_->indexCount_), sizeof(asset->shape_->indexCount_));
    asset->shape_->indexData_.resize(asset->shape_->indexCount_);
    input.read((char*)asset->shape_->indexData_.data(), sizeof(unsigned) * asset->shape_->indexCount_);

    return true;
}

}
