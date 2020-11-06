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
#include "CreateSceneAction.h"
#include <abscommon/FileUtils.h>
#include <abscommon/StringUtils.h>
#include <sa/StringTempl.h>
#include <AB/CommonConfig.h>
#include <abscommon/Process.hpp>
#include <codecvt>
#include <sa/Process.h>
#include <sa/StringHash.h>
#include <sa/Assert.h>
#include <absmath/BoundingBox.h>
#include <absmath/Sphere.h>
#include <absmath/Transformation.h>
#include "CreateHeightMapAction.h"

void CreateSceneAction::Execute()
{
    if (!Utils::EnsureDirectory(outputDirectory_))
    {
        std::cerr << "Error creating directory " << outputDirectory_ << std::endl;
        return;
    }
    searchpaths_.push_back(Utils::ExtractFileDir(file_) + "/");
    // AbData
    searchpaths_.push_back(Utils::ExtractFileDir(file_) + "/../");
    searchpaths_.push_back(Utils::ExtractFileDir(file_) + "/../../Data/");
    if (!LoadScene())
    {
        std::cerr << "Error loading scene " << file_ << std::endl;
        return;
    }
    if (!CopySceneFile())
    {
        std::cerr << "Error copying Scene file " << file_ << " to " << outputDirectory_ << std::endl;
    }
    if (!SaveObstacles())
    {
        std::cerr << "Error saving obstacles" << std::endl;
    }
    if (!CreateHightmap())
    {
        std::cerr << "Error creating height map" << std::endl;
    }
    if (!CreateNavMesh())
    {
        std::cerr << "Error creating navigation mesh" << std::endl;
    }
    if (!CreateIndexFile())
    {
        std::cerr << "Error creating index.xml" << std::endl;
    }
}

bool CreateSceneAction::CopySceneFile()
{
    if (outputDirectory_.empty())
    {
        std::cerr << "Output directory is empty" << std::endl;
        return false;
    }
    std::cout << "Copy scene file " << file_ << " to " << outputDirectory_ << std::endl;
    std::string fileName = Utils::ConcatPath(outputDirectory_, sa::ExtractFileName<char>(file_));
    return Utils::FileCopy(file_, fileName);
}

bool CreateSceneAction::CreateHightmap()
{
    if (outputDirectory_.empty())
    {
        std::cerr << "Output directory is empty" << std::endl;
        return false;
    }
    std::string fileName = Utils::ConcatPath(outputDirectory_, sa::ExtractFileName<char>(heightfieldFile_));
    std::cout << "Copying heightmap " << heightfieldFile_ << std::endl;
    if (!Utils::FileCopy(heightfieldFile_, fileName))
        return false;

    CreateHeightMapAction action(heightfieldFile_, outputDirectory_);
    action.spacing_ = heightmapSpacing_;
    action.patchSize_ = patchSize_;
    action.Execute();
    return true;
}

bool CreateSceneAction::SaveObstacles()
{
    std::string fileName = Utils::ConcatPath(outputDirectory_, sa::ExtractFileName<char>(heightfieldFile_) + ".obstacles");
    std::fstream f(fileName, std::fstream::out | std::fstream::binary);
    if (!f.is_open())
        return false;

    size_t count = obstackles_.size();
    std::cout << "Saving " << count << " obstacles to " << fileName << std::endl;

    f.write((char*)&count, sizeof(uint64_t));
    for (const auto& o : obstackles_)
    {
        size_t vertexCount = o->vertexCount_;
        f.write((char*)&vertexCount, sizeof(uint64_t));
        for (const auto& v : o->vertexData_)
        {
            f.write((char*)&v.x_, sizeof(float));
            f.write((char*)&v.y_, sizeof(float));
            f.write((char*)&v.z_, sizeof(float));
        }

        size_t indexCount = o->indexCount_;
        f.write((char*)&indexCount, sizeof(uint64_t));
        for (const auto& i : o->indexData_)
        {
            f.write((char*)&i, sizeof(int));
        }
    }

    return true;
}

bool CreateSceneAction::CreateNavMesh()
{
    navmeshFile_ = heightfieldFile_ + ".navmesh";

    std::cout << "Creating navigation mesh from " << heightfieldFile_ << std::endl;

    // Run genavmesh
    std::stringstream ss;

    ss << Utils::ConcatPath(sa::Process::GetSelfPath(), "genavmesh");
    ss << " -createobj ";
    ss << "-hmsx:" << heightmapSpacing_.x_ << " ";
    ss << "-hmsy:" << heightmapSpacing_.y_ << " ";
    ss << "-hmsz:" << heightmapSpacing_.z_ << " ";
    ss << "-hmps:" << patchSize_ << " ";
    std::string fileName = Utils::ConcatPath(outputDirectory_, sa::ExtractFileName<char>(heightfieldFile_));
    ss << Utils::EscapeArguments(fileName);

    const std::string cmdLine = ss.str();
    std::cout << "Running commandline: " << cmdLine << std::endl;
#ifdef AB_WINDOWS
#if defined(UNICODE)
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wcmdLine = converter.from_bytes(cmdLine);
    System::Process process(wcmdLine);
#else
    System::Process process(cmdLine);
#endif
#else
    System::Process process(cmdLine);
#endif
    int exitCode = process.get_exit_status();
    return exitCode == 0;
}

bool CreateSceneAction::CreateIndexFile()
{
    pugi::xml_document doc;
    auto declarationNode = doc.append_child(pugi::node_declaration);
    declarationNode.append_attribute("version").set_value("1.0");
    declarationNode.append_attribute("encoding").set_value("UTF-8");
    declarationNode.append_attribute("standalone").set_value("yes");
    auto root = doc.append_child("index");
    {
        auto nd = root.append_child("file");
        nd.append_attribute("type").set_value("Scene");
        nd.append_attribute("src").set_value(Utils::ExtractFileName(file_).c_str());
    }
    {
        auto nd = root.append_child("file");
        nd.append_attribute("type").set_value("NavMesh");
        nd.append_attribute("src").set_value(Utils::ExtractFileName(navmeshFile_).c_str());
    }
    {
        auto nd = root.append_child("file");
        nd.append_attribute("type").set_value("Terrain");
        nd.append_attribute("src").set_value(Utils::ExtractFileName(heightfieldFile_ + ".hm").c_str());
    }

    std::string fileName = Utils::ConcatPath(outputDirectory_, "index.xml");
    std::cout << "Creating index.xml" << std::endl;

    std::fstream f(fileName, std::fstream::out);
    if (!f.is_open())
        return false;
    doc.save(f);
    return true;
}

std::string CreateSceneAction::FindFile(const std::string& name)
{
    if (Utils::FileExists(name))
        return name;

    for (const auto& p : searchpaths_)
    {
        std::string f = Utils::ConcatPath(p, name);
        if (Utils::FileExists(f))
            return f;
    }
    return "";
}

static std::string GetComponentAttribute(const pugi::xml_node& node, const std::string& name)
{
    for (const auto& attr : node.children("attribute"))
    {
        const pugi::xml_attribute nameAttr = attr.attribute("name");
        const pugi::xml_attribute valueAttr = attr.attribute("value");
        if (name.compare(nameAttr.as_string()) == 0)
        {
            return valueAttr.as_string();
        }
    }
    return "";
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

static std::vector<VertexElement> GetElements(unsigned mask)
{
    std::vector<VertexElement> result;
    for (unsigned i = 0; i < 14; ++i)
    {
        if (mask & (1u << i))
            result.push_back(LEGACY_VERTEXELEMENTS[i]);
    }

    return result;
}

static unsigned GetVertexSize(const std::vector<VertexElement>& elements)
{
    unsigned size = 0;

    for (unsigned i = 0; i < elements.size(); ++i)
        size += ELEMENT_TYPESIZES[elements[i].type_];

    return size;

}

static bool LoadUrhoModel(const std::string& filename, Math::Shape& result)
{
    //return true;
    std::fstream input(filename, std::ios::binary | std::fstream::in);
    if (!input.is_open())
        return false;

    std::string fileId;
    fileId.resize(4);
    input.read(fileId.data(), 4);
    if (fileId != "UMDL" && fileId != "UMD2")
    {
        std::cerr << filename << " is not a valid model file" << std::endl;
        return false;
    }
    bool hasVertexDeclarations = (fileId == "UMD2");
    unsigned numVertexBuffers = 0;
    input.read((char*)&numVertexBuffers, sizeof(unsigned));

    if (numVertexBuffers != 1)
    {
        std::cerr << "Model contains " << numVertexBuffers << " vertex buffers" << std::endl;
        return false;
    }

    std::vector<VertexElement> elements;
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

        auto* buff = new unsigned char[vertexCount * vertexSize];

        input.read((char*)buff, vertexCount * vertexSize);
        for (unsigned i = 0; i < vertexCount * vertexSize; i += vertexSize)
        {
            float* vertices = (float*)(buff + i);
            result.vertexData_.push_back({ vertices[0], vertices[1], vertices[2] });
        }
        result.vertexCount_ = vertexCount;

        delete[] buff;
    }

    unsigned numIndexBuffers = 0;
    input.read((char*)&numIndexBuffers, sizeof(unsigned));
    if (numIndexBuffers != 1)
    {
        std::cerr << "Model contains " << numIndexBuffers << " index buffers" << std::endl;
        return false;
    }

    unsigned indexCount = 0;
    input.read((char*)&indexCount, sizeof(unsigned));
    unsigned indexSize = 0;
    input.read((char*)&indexSize, sizeof(unsigned));

    {
        auto* buff = new unsigned char[indexCount * indexSize];
        input.read((char*)buff, indexCount * indexSize);

        result.indexData_.reserve(indexCount);
        if (indexSize == sizeof(unsigned))
        {
            unsigned* indices = (unsigned*)buff;
            for (unsigned i = 0; i < indexCount; ++i)
            {
                result.indexData_.push_back(indices[i]);
            }
        }
        else
        {
            unsigned short* indices = (unsigned short*)buff;
            for (unsigned i = 0; i < indexCount; ++i)
            {
                result.indexData_.push_back(indices[i]);
            }
        }

        delete[] buff;
        result.indexCount_ = indexCount;
    }

//    result.SaveToOBJ(R"(c:\Users\Stefan Ascher\Documents\Visual Studio 2015\Projects\ABx\bin\test\test.obj)");
    return true;
}

bool CreateSceneAction::SaveModel(const Math::Shape& shape, const std::string& filename)
{
    if (dataDir_.empty())
        return false;

    // When the bounding colume is triangle mesh or convex hull also the server needs the model.
    // Let's save the model to a simplified file format with the same name in the servers data directory.
    std::string outfile = Utils::ConcatPath(dataDir_, filename);
    std::string outdir = Utils::ExtractFileDir(outfile);
    if (!Utils::EnsureDirectory(outdir))
        return false;
    std::fstream f(outfile, std::fstream::out | std::fstream::binary);
    if (!f.is_open())
        return false;

    std::cout << "Saving model file as " << outfile << std::endl;

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


bool CreateSceneAction::LoadSceneNode(const pugi::xml_node& node)
{
    using namespace sa::literals;

    Math::Transformation transform;

    Math::Vector3 size = Math::Vector3::One;
    Math::Vector3 offset = Math::Vector3::Zero;
    Math::Quaternion offsetRot = Math::Quaternion::Identity;

    for (const auto& attr : node.children("attribute"))
    {
        const pugi::xml_attribute nameAttr = attr.attribute("name");
        const pugi::xml_attribute valueAttr = attr.attribute("value");
        const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
        switch (nameHash)
        {
        case "Position"_Hash:
            transform.position_ = Math::Vector3(valueAttr.as_string());
            break;
        case "Rotation"_Hash:
            transform.oriention_ = Math::Quaternion(valueAttr.as_string()).Normal();
            break;
        case "Scale"_Hash:
            transform.scale_ = Math::Vector3(valueAttr.as_string());
            break;
        }
    }

    // If we have a rigid body collide by default with everything. That's also Urho3Ds default.
    uint32_t colisionMask = 0xFFFFFFFF;

    for (const auto& comp : node.children("component"))
    {
        const pugi::xml_attribute type_attr = comp.attribute("type");
        const size_t type_hash = sa::StringHashRt(type_attr.as_string());
        switch (type_hash)
        {
        case "CollisionShape"_Hash:
        {
            size_t collShape = "Box"_Hash;
            for (const auto& attr : comp.children())
            {
                const pugi::xml_attribute& nameAttr = attr.attribute("name");
                const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                const pugi::xml_attribute& valueAttr = attr.attribute("value");
                const size_t valueHash = sa::StringHashRt(valueAttr.as_string());
                switch (nameHash)
                {
                case "Size"_Hash:
                    size = Math::Vector3(valueAttr.as_string());
                    break;
                case "Offset Position"_Hash:
                    offset = Math::Vector3(valueAttr.as_string());
                    break;
                case "Offset Rotation"_Hash:
                    offsetRot = Math::Quaternion(valueAttr.as_string()).Normal();
                    break;
                case "Shape Type"_Hash:
                {
                    collShape = valueHash;
                }
                }
            }

            if (collShape == "Box"_Hash && size != Math::Vector3::Zero)
            {
                // The object has the scaling.
                const Math::Vector3 halfSize = (size * 0.5f);
                Math::BoundingBox bb(offset - halfSize, offset + halfSize);
                // Add Node and Offset rotation -> absolute orientation
                transform.oriention_ = transform.oriention_ * offsetRot;
                const Math::Matrix4 matrix = static_cast<Math::Matrix4>(transform.GetMatrix());
                Math::Shape shape = bb.GetShape();
                for (auto& v : shape.vertexData_)
                    v = matrix * v;
                obstackles_.push_back(std::make_unique<Math::Shape>(std::move(shape)));
            }
            else if ((collShape == "Sphere"_Hash || collShape == "Cylinder"_Hash) &&
                size != Math::Vector3::Zero)
            {
                // The object has the scaling.
                float radius = (size.x_ * 0.5f);
                Math::Sphere sphere(offset, radius);
                transform.oriention_ = transform.oriention_ * offsetRot;
                const Math::Matrix4 matrix = static_cast<Math::Matrix4>(transform.GetMatrix());
                Math::Shape shape = sphere.GetShape();
                for (auto& v : shape.vertexData_)
                    v = matrix * v;
                obstackles_.push_back(std::make_unique<Math::Shape>(std::move(shape)));
            }
            else if (collShape == "TriangleMesh"_Hash || collShape == "ConvexHull"_Hash)
            {
                std::string model = GetComponentAttribute(comp, "Model");
                if (!model.empty())
                {
                    // Model;Models/ResurrectionShrine1.mdl
                    auto parts = sa::Split(model, ";", false, false);
                    if (parts.size() != 2)
                    {
                        std::cerr << "Wrong format of attribute value " << model << std::endl;
                        return false;
                    }
                    std::string modelFile = FindFile(parts[1]);
                    if (!modelFile.empty())
                    {
                        Math::Shape modelShape;
                        if (LoadUrhoModel(modelFile, modelShape))
                        {
                            SaveModel(modelShape, parts[1]);
                            const Math::Matrix4 matrix = static_cast<Math::Matrix4>(transform.GetMatrix());
                            for (auto& v : modelShape.vertexData_)
                                v = matrix * v;
                            obstackles_.push_back(std::make_unique<Math::Shape>(std::move(modelShape)));
                        }
                        else
                            std::cerr << "Error loading model " << modelFile << std::endl;
                    }
                    else
                        std::cerr << "File " << parts[1] << " not found" << std::endl;
                }
                else
                    std::cerr << "Missing Model attribute" << std::endl;
            }
            break;
        }
        case "Terrain"_Hash:
        {
            for (const auto& attr : comp.children())
            {
                const pugi::xml_attribute& nameAttr = attr.attribute("name");
                const size_t nameHash = sa::StringHashRt(nameAttr.as_string());
                const pugi::xml_attribute& valueAttr = attr.attribute("value");
                switch (nameHash)
                {
                case "Vertex Spacing"_Hash:
                    heightmapSpacing_ = Math::Vector3(valueAttr.as_string());
                    break;
                case "Patch Size"_Hash:
                    patchSize_ = valueAttr.as_int();
                    break;
                case "Height Map"_Hash:
                {
                    // value="Image;Textures/Rhodes_Heightfield.png"
                    auto parts = sa::Split(valueAttr.as_string(), ";", false, false);
                    if (parts.size() != 2)
                    {
                        std::cerr << "Wrong format of attribute value " << valueAttr.as_string() << std::endl;
                        return false;
                    }
                    heightfieldFile_ = FindFile(parts[1]);
                    if (heightfieldFile_.empty())
                    {
                        std::cerr << "File " << parts[1] << " not found" << std::endl;
                        return false;
                    }
                    break;
                }
                }
            }
        }
        }
    }
    return true;
}

bool CreateSceneAction::LoadScene()
{
    patchSize_ = 32;
    std::cout << "Loading scene " << file_ << std::endl;
    pugi::xml_document doc;
    const pugi::xml_parse_result result = doc.load_file(file_.c_str());
    if (result.status != pugi::status_ok)
    {
        std::cerr << "Error loading file " << file_ << ": " << result.description() << std::endl;
        return false;
    }
    const pugi::xml_node sceneNode = doc.child("scene");
    if (!sceneNode)
    {
        std::cerr << "File " << file_ << " does not have a scene node" << std::endl;
        return false;
    }
    for (pugi::xml_node_iterator it = sceneNode.begin(); it != sceneNode.end(); ++it)
    {
        if (strcmp((*it).name(), "node") == 0)
        {
            if (!LoadSceneNode(*it))
            {
                std::cerr << "Error loading scene node" << std::endl;
                // Can't continue
                return false;
            }
        }
    }
    return true;
}
