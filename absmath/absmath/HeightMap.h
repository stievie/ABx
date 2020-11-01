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

#include "Point.h"
#include "Vector3.h"
#include "BoundingBox.h"
#include "Point.h"
#include "Matrix4.h"
#include <eastl.hpp>

namespace Math {

class Shape;

class HeightMap
{
private:
    /// Transformation matrix
    Matrix4 matrix_ = Matrix4::Identity;
    Matrix4 inverseMatrix_;
public:
    HeightMap();
    HeightMap(const HeightMap& other) :
        matrix_(other.matrix_),
        spacing_(other.spacing_),
        patchSize_(other.patchSize_),
        minHeight_(other.minHeight_),
        maxHeight_(other.maxHeight_),
        numVertices_(other.numVertices_),
        numPatches_(other.numPatches_),
        patchWorldSize_(other.patchWorldSize_),
        patchWorldOrigin_(other.patchWorldOrigin_),
        heightData_(other.heightData_),
        boundingBox_(other.boundingBox_)
    {}
    HeightMap(HeightMap&& other) noexcept :
        matrix_(std::move(other.matrix_)),
        spacing_(std::move(other.spacing_)),
        patchSize_(other.patchSize_),
        minHeight_(other.minHeight_),
        maxHeight_(other.maxHeight_),
        numVertices_(std::move(other.numVertices_)),
        numPatches_(std::move(other.numPatches_)),
        patchWorldSize_(std::move(other.patchWorldSize_)),
        patchWorldOrigin_(std::move(other.patchWorldOrigin_)),
        heightData_(std::move(other.heightData_)),
        boundingBox_(std::move(other.boundingBox_))
    {}
    ~HeightMap() = default;

    HeightMap& operator= (const HeightMap& other)
    {
        if (this != &other)
        {
            spacing_ = other.spacing_;
            patchSize_ = other.patchSize_;
            minHeight_ = other.minHeight_;
            maxHeight_ = other.maxHeight_;
            numVertices_ = other.numVertices_;
            numPatches_ = other.numPatches_;
            patchWorldSize_ = other.patchWorldSize_;
            patchWorldOrigin_ = other.patchWorldOrigin_;
            heightData_ = other.heightData_;
            boundingBox_ = other.boundingBox_;
            matrix_ = other.matrix_;
        }
        return *this;
    }
    HeightMap& operator= (HeightMap&& other) noexcept
    {
        spacing_ = std::move(other.spacing_);
        patchSize_ = other.patchSize_;
        minHeight_ = other.minHeight_;
        maxHeight_ = other.maxHeight_;
        numVertices_ = std::move(other.numVertices_);
        numPatches_ = std::move(other.numPatches_);
        patchWorldSize_ = std::move(other.patchWorldSize_);
        patchWorldOrigin_ = std::move(other.patchWorldOrigin_);
        heightData_ = std::move(other.heightData_);
        boundingBox_ = std::move(other.boundingBox_);
        matrix_ = std::move(other.matrix_);
        return *this;
    }
    void ProcessData();

    float GetRawHeight(int x, int z) const;
    Vector3 GetRawNormal(int x, int z) const;
    /// Return height at world coordinates.
    float GetHeight(const Vector3& world) const;
    Vector3 GetNormal(const Vector3& world) const;

    BoundingBox GetBoundingBox() const
    {
        return boundingBox_;
    }
    const HeightMap& Transformed(const Matrix4&) const
    {
        return *this;
    }
    void SetMatrix(const Matrix4& matrix);

    bool Collides(const Sphere& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const BoundingBox& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const ConvexHull& b2, const Vector3& velocity, Vector3& move) const;
    bool Collides(const HeightMap&, const Vector3&, Vector3&) const;

    Shape GetShape() const;

    Point<int> WorldToHeightmap(const Vector3& world);
    Vector3 HeightmapToWorld(const Point<int>& pixel);
    int GetWidth() const
    {
        return numVertices_.x_;
    }
    int GetHeight() const
    {
        return numVertices_.y_;
    }

    /// Vertex and height spacing.
    Vector3 spacing_;
    int32_t patchSize_;
    float minHeight_;
    float maxHeight_;
    Point<int32_t> numVertices_;
    Point<int32_t> numPatches_;
    Point<float> patchWorldSize_;
    Point<float> patchWorldOrigin_;

    ea::vector<float> heightData_;
    BoundingBox boundingBox_;
};

}

