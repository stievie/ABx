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

#include "HeightMap.h"

void HeightMap::RegisterObject(Context* context)
{
    context->RegisterFactory<HeightMap>();
}

HeightMap::HeightMap(Context* context) :
    Component(context)
{
    inverseMatrix_ = matrix_.Inverse();
}

HeightMap::~HeightMap() = default;

void HeightMap::LoadHeightmap(JSONFile* file)
{
    const auto& root = file->GetRoot();
    {
        const auto& val = root.Get("maxHeight");
        if (val.IsNumber())
            maxHeight_ = val.GetFloat();
    }
    {
        const auto& val = root.Get("minHeight");
        if (val.IsNumber())
            minHeight_ = val.GetFloat();
    }
    {
        const auto& valx = root.Get("numPatchesX");
        const auto& valy = root.Get("numPatchesY");
        if (valx.IsNumber() && valy.IsNumber())
            numPatches_ = { valx.GetInt(), valy.GetInt() };
    }
    {
        const auto& valx = root.Get("numVerticesX");
        const auto& valy = root.Get("numVerticesY");
        if (valx.IsNumber() && valy.IsNumber())
            numVertices_ = { valx.GetInt(), valy.GetInt() };
    }
    {
        const auto& val = root.Get("patchSize");
        if (val.IsNumber())
            patchSize_ = val.GetInt();
    }
    {
        const auto& valx = root.Get("patchWorldOriginX");
        const auto& valy = root.Get("patchWorldOriginY");
        if (valx.IsNumber() && valy.IsNumber())
            patchWorldOrigin_ = { valx.GetFloat(), valy.GetFloat() };
    }
    {
        const auto& valx = root.Get("patchWorldSizeX");
        const auto& valy = root.Get("patchWorldSizeY");
        if (valx.IsNumber() && valy.IsNumber())
            patchWorldSize_ = { valx.GetFloat(), valy.GetFloat() };
    }
    {
        const auto& val = root.Get("nDataValues");
        if (val.IsNumber())
        {
            heightData_ = new float[val.GetInt()];
            const auto& valData = root.Get("data");
            if (valData.IsArray())
            {
                const auto& dataArr = valData.GetArray();
                for (unsigned i = 0; i < dataArr.Size(); ++i)
                {
                    const auto& dataVal = dataArr[i];
                    if (dataVal.IsNumber())
                        heightData_[i] = dataVal.GetFloat();
                    else
                        heightData_[i] = -M_INFINITY;
                }
            }
        }
    }
}

float HeightMap::GetRawHeight(int x, int z) const
{
    if (!heightData_)
        return 0.0f;
    const int _x = Clamp(x, 0, numVertices_.x_ - 1);
    const int _z = Clamp(z, 0, numVertices_.y_ - 1);
    const size_t offset = static_cast<size_t>((size_t)_z * (size_t)numVertices_.x_ + (size_t)_x);
    return heightData_[offset];
}

Vector3 HeightMap::GetRawNormal(int x, int z) const
{
    const float baseHeight = GetRawHeight(x, z);
    const float nSlope = GetRawHeight(x, z - 1) - baseHeight;
    const float neSlope = GetRawHeight(x + 1, z - 1) - baseHeight;
    const float eSlope = GetRawHeight(x + 1, z) - baseHeight;
    const float seSlope = GetRawHeight(x + 1, z + 1) - baseHeight;
    const float sSlope = GetRawHeight(x, z + 1) - baseHeight;
    const float swSlope = GetRawHeight(x - 1, z + 1) - baseHeight;
    const float wSlope = GetRawHeight(x - 1, z) - baseHeight;
    const float nwSlope = GetRawHeight(x - 1, z - 1) - baseHeight;
    const float up = 0.5f * (spacing_.x_ + spacing_.z_);

    return (Vector3(0.0f, up, nSlope) +
        Vector3(-neSlope, up, neSlope) +
        Vector3(-eSlope, up, 0.0f) +
        Vector3(-seSlope, up, -seSlope) +
        Vector3(0.0f, up, -sSlope) +
        Vector3(swSlope, up, -swSlope) +
        Vector3(wSlope, up, 0.0f) +
        Vector3(nwSlope, up, nwSlope)).Normalized();
}

float HeightMap::GetHeight(const Vector3& world) const
{
    // Get local
    const Vector3 position = inverseMatrix_ * world;
    const float xPos = (position.x_ - patchWorldOrigin_.x_) / spacing_.x_;
    const float zPos = (position.z_ - patchWorldOrigin_.y_) / spacing_.z_;
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    const unsigned uxPos = static_cast<unsigned>(xPos);
    const unsigned uzPos = static_cast<unsigned>(zPos);
    float h1, h2, h3;

    if (xFrac + zFrac >= 1.0f)
    {
        h1 = GetRawHeight(uxPos + 1, uzPos + 1);
        h2 = GetRawHeight(uxPos, uzPos + 1);
        h3 = GetRawHeight(uxPos + 1, uzPos);
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        h1 = GetRawHeight(uxPos, uzPos);
        h2 = GetRawHeight(uxPos + 1, uzPos);
        h3 = GetRawHeight(uxPos, uzPos + 1);
    }

    // 2nd Layer may have -inf for undefined heights
    if (IsInf(h1) || IsInf(h2) || IsInf(h3))
        return -M_INFINITY;

    const float h = h1 * (1.0f - xFrac - zFrac) + h2 * xFrac + h3 * zFrac;

    return matrix_.Scale().y_ * h + matrix_.Translation().y_;
}

Vector3 HeightMap::GetNormal(const Vector3& world) const
{
    const Vector3 position = inverseMatrix_ * world;
    const float xPos = (position.x_ / spacing_.x_) + ((float)numVertices_.x_ / 2.0f);
    const float zPos = (position.z_ / spacing_.z_) + ((float)numVertices_.y_ / 2.0f);
    float xFrac = Fract(xPos);
    float zFrac = Fract(zPos);
    Vector3 n1, n2, n3;

    if (xFrac + zFrac >= 1.0f)
    {
        n1 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos + 1);
        n2 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
        n3 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
        xFrac = 1.0f - xFrac;
        zFrac = 1.0f - zFrac;
    }
    else
    {
        n1 = GetRawNormal((unsigned)xPos, (unsigned)zPos);
        n2 = GetRawNormal((unsigned)xPos + 1, (unsigned)zPos);
        n3 = GetRawNormal((unsigned)xPos, (unsigned)zPos + 1);
    }

    const Vector3 n = (n1 * (1.0f - xFrac - zFrac) + n2 * xFrac + n3 * zFrac).Normalized();
    return matrix_.Rotation() * n;
}

IntVector2 HeightMap::WorldToHeightmap(const Vector3& world)
{
    const Vector3 pos = inverseMatrix_ * world;
    int xPos = static_cast<int>(pos.x_ / spacing_.x_ + 0.5f);
    int zPos = static_cast<int>(pos.z_ / spacing_.z_ + 0.5f);
    xPos = Clamp(xPos, 0, numVertices_.x_ - 1);
    zPos = Clamp(zPos, 0, numVertices_.y_ - 1);
    return IntVector2(xPos, zPos);
}

Vector3 HeightMap::HeightmapToWorld(const IntVector2& pixel)
{
    const IntVector2 pos(pixel.x_, numVertices_.y_ - 1 - pixel.y_);
    const float xPos = static_cast<float>(pos.x_ * spacing_.x_);
    const float zPos = static_cast<float>(pos.y_ * spacing_.z_);
    const Vector3 lPos(xPos, 0.0f, zPos);
    Vector3 wPos = matrix_ * lPos;
    wPos.y_ = GetHeight(wPos);

    return wPos;
}
