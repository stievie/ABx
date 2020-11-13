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

#include "stdafx.h"
#include "IO.h"
#include <fstream>
#include <iomanip>
#include <istream>
#include <sstream>

namespace IO {

bool LoadShape(const std::string& filename, Math::Shape& shape, Math::BoundingBox& bb)
{
    std::fstream input(filename, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    char sig[4];
    input.read(sig, 4);
    if (sig[0] != 'M' || sig[1] != 'O' || sig[2] != 'D' || sig[3] != 'L')
        return false;

    // Read bounding box
    input.read((char*)bb.min_.Data(), sizeof(float) * 3);
    input.read((char*)bb.max_.Data(), sizeof(float) * 3);

    uint32_t vertexCount = 0;
    input.read((char*)&vertexCount, sizeof(uint32_t));
    shape.vertexCount_ = vertexCount;
    shape.vertexData_.resize(shape.vertexCount_);
    for (uint32_t i = 0; i < vertexCount; ++i)
        input.read((char*)shape.vertexData_[i].Data(), sizeof(float) * 3);

    uint32_t indexCount = 0;
    input.read((char*)&indexCount, sizeof(uint32_t));
    shape.indexCount_ = indexCount;
    shape.indexData_.reserve(shape.indexCount_);
    for (uint32_t i = 0; i < indexCount; ++i)
    {
        uint32_t index = 0;
        input.read((char*)&index, sizeof(uint32_t));
        shape.indexData_.push_back(index);
    }

    return true;
}

bool SaveShape(const std::string& filename, const Math::Shape& shape)
{
    std::fstream f(filename, std::fstream::out | std::fstream::binary);
    if (!f.is_open())
        return false;

    Math::BoundingBox bb;
    bb.Merge(&shape.vertexData_[0], shape.vertexData_.size());

    f.write("MODL", 4);
    f.write((char*)bb.min_.Data(), sizeof(float) * 3);
    f.write((char*)bb.max_.Data(), sizeof(float) * 3);

    f.write((char*)&shape.vertexCount_, sizeof(uint32_t));
    f.write((char*)shape.vertexData_.data(), sizeof(float) * 3 * shape.vertexCount_);

    f.write((char*)&shape.indexCount_, sizeof(uint32_t));
    for (size_t i = 0; i < shape.indexCount_; ++i)
        f.write((char*)&shape.indexData_[i], sizeof(uint32_t));
    return true;
}

bool SaveShapeToOBJ(const std::string& filename, const Math::Shape& shape)
{
    std::fstream f(filename, std::fstream::out);
    f << std::fixed << std::setprecision(6);
    f << "o " << "terrain" << std::endl;
    f << "# vertices " << shape.vertexCount_ << std::endl;
    for (const auto& v : shape.vertexData_)
    {
        f << "v " << v.x_ << " " << v.y_ << " " << v.z_ << std::endl;
    }
    f << "# triangles " << shape.indexCount_ / 3 << std::endl;
    for (size_t i = 0; i < shape.indexCount_; i += 3)
    {
        f << "f " << shape.indexData_[i] + 1 << " " << shape.indexData_[i + 1] + 1 << " " << shape.indexData_[i + 2] + 1 << std::endl;
    }

    f << "# EOF" << std::endl;
    f.close();
    return true;
}

bool LoadShapeFromOBJ(const std::string& filename, Math::Shape& shape)
{
    std::ifstream in(filename);
    if (!in.is_open())
        return false;

    std::istringstream lineStream;
    char line[1024];
    std::string op;
    while (in.good())
    {
        in.getline(line, 1023);
        lineStream.clear();
        lineStream.str(line);

        if (!(lineStream >> op))
            continue;

        if (op == "v")
        {
            Math::Vector3 vertex;
            lineStream >> vertex.x_ >> vertex.y_ >> vertex.z_;
            shape.vertexData_.push_back(std::move(vertex));
            ++shape.vertexCount_;
        }
        else if (op == "vt")
        {
        }
        else if (op == "vn")
        {
        }
        else if (op == "g")
        {
        }
        else if (op == "f")
        {
            unsigned i1, i2, i3;
            lineStream >> i1 >> i2 >> i3;
            // Our indices are 0-based
            shape.AddTriangle(i1 - 1, i2 - 1, i3 - 1);
        }
    }

    return true;
}

enum VertexElementType
{
    TYPE_INT = 0,
    TYPE_FLOAT,
    TYPE_VECTOR2,
    TYPE_VECTOR3,
    TYPE_VECTOR4,
    TYPE_UBYTE4,
    TYPE_UBYTE4_NORM,
    MAX_VERTEX_ELEMENT_TYPES
};

enum VertexElementSemantic
{
    SEM_POSITION = 0,
    SEM_NORMAL,
    SEM_BINORMAL,
    SEM_TANGENT,
    SEM_TEXCOORD,
    SEM_COLOR,
    SEM_BLENDWEIGHTS,
    SEM_BLENDINDICES,
    SEM_OBJECTINDEX,
    MAX_VERTEX_ELEMENT_SEMANTICS
};

struct VertexElement
{
    /// Data type of element.
    VertexElementType type_;
    /// Semantic of element.
    VertexElementSemantic semantic_;
    /// Semantic index of element, for example multi-texcoords.
    unsigned char index_;
    /// Per-instance flag.
    bool perInstance_;
    /// Offset of element from vertex start. Filled by VertexBuffer once the vertex declaration is built.
    unsigned offset_;
};

extern const VertexElement LEGACY_VERTEXELEMENTS[] =
{
    { TYPE_VECTOR3, SEM_POSITION, 0, false, 0 },     // Position
    { TYPE_VECTOR3, SEM_NORMAL, 0, false, 0 },       // Normal
    { TYPE_UBYTE4_NORM, SEM_COLOR, 0, false, 0 },    // Color
    { TYPE_VECTOR2, SEM_TEXCOORD, 0, false, 0 },     // Texcoord1
    { TYPE_VECTOR2, SEM_TEXCOORD, 1, false, 0 },     // Texcoord2
    { TYPE_VECTOR3, SEM_TEXCOORD, 0, false, 0 },     // Cubetexcoord1
    { TYPE_VECTOR3, SEM_TEXCOORD, 1, false, 0 },     // Cubetexcoord2
    { TYPE_VECTOR4, SEM_TANGENT, 0, false, 0 },      // Tangent
    { TYPE_VECTOR4, SEM_BLENDWEIGHTS, 0, false, 0 }, // Blendweights
    { TYPE_UBYTE4, SEM_BLENDINDICES, 0, false, 0 },  // Blendindices
    { TYPE_VECTOR4, SEM_TEXCOORD, 4, true, 0 },      // Instancematrix1
    { TYPE_VECTOR4, SEM_TEXCOORD, 5, true, 0 },      // Instancematrix2
    { TYPE_VECTOR4, SEM_TEXCOORD, 6, true, 0 },      // Instancematrix3
    { TYPE_INT, SEM_OBJECTINDEX, 0, false, 0 }       // Objectindex
};

const unsigned ELEMENT_TYPESIZES[] =
{
    sizeof(int),
    sizeof(float),
    2 * sizeof(float),
    3 * sizeof(float),
    4 * sizeof(float),
    sizeof(unsigned),
    sizeof(unsigned)
};

static ea::vector<VertexElement> GetElements(unsigned mask)
{
    ea::vector<VertexElement> result;
    for (unsigned i = 0; i < 14; ++i)
    {
        if (mask & (1u << i))
            result.push_back(LEGACY_VERTEXELEMENTS[i]);
    }

    return result;
}

static unsigned GetVertexSize(const ea::vector<VertexElement>& elements)
{
    unsigned size = 0;

    for (unsigned i = 0; i < elements.size(); ++i)
        size += ELEMENT_TYPESIZES[elements[i].type_];

    return size;
}

bool LoadUrhoModel(const std::string& filename, Math::Shape& shape)
{
    std::fstream input(filename, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    std::string fileId;
    fileId.resize(4);
    input.read(fileId.data(), 4);
    if (fileId != "UMDL" && fileId != "UMD2")
    {
        return false;
    }
    bool hasVertexDeclarations = (fileId == "UMD2");
    unsigned numVertexBuffers = 0;
    input.read((char*)&numVertexBuffers, sizeof(unsigned));

    if (numVertexBuffers != 1)
    {
        return false;
    }

    ea::vector<VertexElement> elements;
    unsigned vertexCount = 0;
    input.read((char*)&vertexCount, sizeof(unsigned));
    if (!hasVertexDeclarations)
    {
        unsigned elementMask = 0;
        input.read((char*)&elementMask, sizeof(unsigned));
        elements = GetElements(elementMask);
    }
    else
    {
        unsigned numElements = 0;
        input.read((char*)&numElements, sizeof(unsigned));
        for (unsigned j = 0; j < numElements; ++j)
        {
            unsigned elementDesc = 0;
            input.read((char*)&elementDesc, sizeof(unsigned));
            auto type = (VertexElementType)(elementDesc & 0xffu);
            auto semantic = (VertexElementSemantic)((elementDesc >> 8u) & 0xffu);
            auto index = (unsigned char)((elementDesc >> 16u) & 0xffu);
            elements.push_back({ type, semantic, index, false, 0 });
        }
    }

    unsigned morphRangeStart = 0;
    input.read((char*)&morphRangeStart, sizeof(unsigned));
    unsigned morphRangeCount = 0;
    input.read((char*)&morphRangeCount, sizeof(unsigned));

    {
        unsigned vertexSize = GetVertexSize(elements);

        size_t buffSize = (size_t)vertexCount * (size_t)vertexSize;
        auto* buff = new unsigned char[buffSize];

        input.read((char*)buff, buffSize);
        for (unsigned i = 0; i < vertexCount * vertexSize; i += vertexSize)
        {
            float* vertices = (float*)(buff + i);
            shape.vertexData_.push_back({ vertices[0], vertices[1], vertices[2] });
        }
        shape.vertexCount_ = vertexCount;

        delete[] buff;
    }

    unsigned numIndexBuffers = 0;
    input.read((char*)&numIndexBuffers, sizeof(unsigned));
    if (numIndexBuffers != 1)
    {
        return false;
    }

    unsigned indexCount = 0;
    input.read((char*)&indexCount, sizeof(unsigned));
    unsigned indexSize = 0;
    input.read((char*)&indexSize, sizeof(unsigned));

    {
        size_t buffSize = (size_t)indexCount * (size_t)indexSize;
        auto* buff = new unsigned char[buffSize];
        input.read((char*)buff, buffSize);

        shape.indexData_.reserve(indexCount);
        if (indexSize == sizeof(unsigned))
        {
            unsigned* indices = (unsigned*)buff;
            for (unsigned i = 0; i < indexCount; ++i)
            {
                shape.indexData_.push_back(indices[i]);
            }
        }
        else
        {
            unsigned short* indices = (unsigned short*)buff;
            for (unsigned i = 0; i < indexCount; ++i)
            {
                shape.indexData_.push_back(indices[i]);
            }
        }

        delete[] buff;
        shape.indexCount_ = indexCount;
    }

    return true;
}

}