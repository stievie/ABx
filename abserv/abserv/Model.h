#pragma once

#include "Asset.h"
#include "Shape.h"
#include "BoundingBox.h"

namespace Game {

/// 3D Model
class Model : public IO::AssetImpl<Model>
{
public:
    Model();
    virtual ~Model();
    Math::BoundingBox GetBoundingBox()
    {
        if (!shape_)
            return Math::BoundingBox();
        if (!boundingBox_.IsDefined())
        {
            boundingBox_.Merge(shape_->vertexData_.data(), shape_->vertexCount_);
        }
        return boundingBox_;
    }

    std::unique_ptr<Math::Shape> shape_;
    Math::BoundingBox boundingBox_;
};

}
