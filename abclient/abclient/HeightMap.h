/**
 * Copyright 2020 Stefan Ascher
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

#include <Urho3DAll.h>

class HeightMap : public Component
{
    URHO3D_OBJECT(HeightMap, Component)
public:
    static void RegisterObject(Context* context);

    HeightMap(Context* context);
    ~HeightMap() override;

    void LoadHeightmap(JSONFile* file);
    float GetRawHeight(int x, int z) const;
    Vector3 GetRawNormal(int x, int z) const;
    /// Return height at world coordinates.
    float GetHeight(const Vector3& world) const;
    Vector3 GetNormal(const Vector3& world) const;
    IntVector2 WorldToHeightmap(const Vector3& world);
    Vector3 HeightmapToWorld(const IntVector2& pixel);

    Vector3 spacing_;
private:
    Matrix4 matrix_ = Matrix4::IDENTITY;
    Matrix4 inverseMatrix_;

    int32_t patchSize_;
    float minHeight_;
    float maxHeight_;
    IntVector2 numVertices_;
    IntVector2 numPatches_;
    Vector2 patchWorldSize_;
    Vector2 patchWorldOrigin_;

    SharedArrayPtr<float> heightData_;
};

