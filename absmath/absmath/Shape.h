/**
 * Copyright 2017-2020 Stefan Ascher
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

#pragma once

#include "Vector3.h"
#include "Matrix4.h"
#include <eastl.hpp>

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
    Shape(Shape&& other) noexcept :
        vertexData_(std::move(other.vertexData_)),
        indexData_(std::move(other.indexData_)),
        vertexCount_(other.vertexCount_),
        indexCount_(other.indexCount_)
    { }
    explicit Shape(const Vector3& vector) :
        vertexCount_(1),
        indexCount_(0)
    {
        vertexData_.push_back(vector);
    }
    explicit Shape(const ea::vector<Vector3>& vertices) :
        vertexData_(vertices),
        vertexCount_(vertexData_.size()),
        indexCount_(0)
    { }
    Shape(const ea::vector<Vector3>& vertices, const ea::vector<size_t>& indices) :
        vertexData_(vertices),
        indexData_(indices),
        vertexCount_(vertexData_.size()),
        indexCount_(indexData_.size())
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
    Shape& operator= (Shape&& other) noexcept
    {
        if (this != &other)
        {
            vertexData_ = std::move(other.vertexData_);
            vertexCount_ = other.vertexCount_;
            indexData_ = std::move(other.indexData_);
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
    /// Naive check whether we have triangles or not.
    bool IsTriangles() const
    {
        if (indexCount_)
            return ((indexCount_ % 3) == 0);
        return (vertexCount_ % 3) == 0;
    }
    size_t GetTriangleCount() const
    {
        if (IsTriangles())
            return GetCount() / 3;
        return 0;
    }
    ea::array<Vector3, 3> GetTriangle(size_t i) const
    {
        ea::array<Vector3, 3> result;
        result[0] = GetVertex(i * 3);
        result[1] = GetVertex(i * 3 + 1);
        result[2] = GetVertex(i * 3 + 2);
        return result;
    }
    /// Check if the triangle face points outside. This only depends on the order of the points.
    bool IsFacingOutside(const ea::array<Vector3, 3>& triangle) const;

    Vector3 GetVertex(size_t index) const
    {
        if (indexCount_)
            return matrix_ * vertexData_[indexData_[index]];
        return matrix_ * vertexData_[index];
    }
    size_t GetCount() const
    {
        if (indexCount_)
            return indexCount_;
        return vertexCount_;
    }
    const float* VertexData() const
    {
        return reinterpret_cast<const float*>(&vertexData_[0]);
    }
    /// Get center position. Note: Shape must be convex.
    Vector3 Center() const;
    size_t VertexDataSize() const { return vertexCount_ * sizeof(float) * 3; }
    /// World Coordinates, that's why we need a transformation matrix
    Vector3 GetFarsetPointInDirection(const Vector3& direction) const;
    Shape Transformed(const Matrix4& transformation) const;

    /// Transformation matrix
    Matrix4 matrix_ = Matrix4::Identity;
    ea::vector<Vector3> vertexData_;
    ea::vector<size_t> indexData_;
    size_t vertexCount_;
    size_t indexCount_;
};

}
