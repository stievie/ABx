#pragma once

#include <iomanip>

class ObjWriter
{
private:
    std::ostream& stream_;
    int vertexIndex_;
public:
    ObjWriter(std::ostream& stream) :
        stream_(stream),
        vertexIndex_(0)
    {
        stream_ << std::fixed << std::setprecision(6);
    }
    ~ObjWriter() = default;

    void Comment(const std::string& comment);
    void MaterialLibrary(const std::string& fileName);
    void Material(const std::string& name);
    void Object(const std::string& name);
    void Group(const std::string& name);
    int Vertex(float x, float y, float z);
    void BeginFace();
    void EndFace();
    void Face(int index);
    ObjWriter& operator << (int vertex)
    {
        Face(vertex);
        return *this;
    }
};

