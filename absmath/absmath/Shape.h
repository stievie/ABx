#pragma once

#include "Vector3.h"
#include "Matrix4.h"

namespace Math {

/// A 3D Mesh
class Shape
{
public:
    Shape() :
        vertexCount_(0),
        indexCount_(0)
    {}
    Shape(const Shape& other) :
        vertexData_(other.vertexData_),
        indexData_(other.indexData_),
        vertexCount_(other.vertexCount_),
        indexCount_(other.indexCount_)
    { }
    explicit Shape(const Vector3& vector) :
        vertexCount_(1),
        indexCount_(0)
    {
        vertexData_.push_back(vector);
    }
    explicit Shape(const std::vector<Vector3>& vertices) :
        vertexData_(vertices),
        vertexCount_((unsigned)vertexData_.size()),
        indexCount_(0)
    { }
    explicit Shape(const std::vector<Vector3>& vertices, const std::vector<unsigned>& indices) :
        vertexData_(vertices),
        indexData_(indices),
        vertexCount_((unsigned)vertexData_.size()),
        indexCount_((unsigned)indexData_.size())
    {}
    ~Shape() = default;

    Shape& operator= (const Shape& other)
    {
        if (this != &other)
        {
            vertexData_ = other.vertexData_;
            vertexCount_ = other.vertexCount_;
            indexData_ = other.indexData_;
            indexCount_ = other.indexCount_;
        }
        return *this;
    }

    bool IsDefined() const
    {
        return vertexCount_ != 0;
    }
    void Reset();

    void AddTriangle(unsigned i1, unsigned i2, unsigned i3);
    const Vector3& GetVertex(unsigned index) const
    {
        if (indexCount_)
            return vertexData_[indexData_[index]];
        return vertexData_[index];
    }
    unsigned GetCount() const
    {
        if (indexCount_)
            return indexCount_;
        return vertexCount_;
    }
    const float* VertexData() const
    {
        return reinterpret_cast<const float*>(&vertexData_[0]);
    }
    size_t VertexDataSize() const { return vertexCount_ * sizeof(float) * 3; }
    /// World Coordinates, that's why we need a transformation matrix
    Vector3 GetFarsetPointInDirection(const Vector3& v) const;

    /// Transformation matrix
    Matrix4 matrix_ = Matrix4::Identity;
    std::vector<Vector3> vertexData_;
    std::vector<unsigned> indexData_;
    unsigned vertexCount_;
    unsigned indexCount_;
};

}
