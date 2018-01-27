#pragma once

#include <iomanip>

class ObjWriter
{
private:
    std::ostream& stream_;
    bool normals_;
public:
    ObjWriter(std::ostream& stream, bool normals = false) :
        stream_(stream),
        normals_(normals)
    {
        stream_ << std::fixed << std::setprecision(6);
    }
    ~ObjWriter() = default;

    void Comment(const std::string& comment);
    void MaterialLibrary(const std::string& fileName);
    void Material(const std::string& name);
    void Object(const std::string& name);
    void Group(const std::string& name);
    void Vertex(float x, float y, float z);
    void Normal(float x, float y, float z);
    void BeginFace();
    void EndFace();
    void Face(int index);
    ObjWriter& operator << (int vertex)
    {
        Face(vertex);
        return *this;
    }
};

