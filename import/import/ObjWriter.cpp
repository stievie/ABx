#include "stdafx.h"
#include "ObjWriter.h"
#include <iostream>

void ObjWriter::Comment(const std::string& comment)
{
    stream_ << "# " << comment << std::endl;
}

void ObjWriter::MaterialLibrary(const std::string& fileName)
{
    stream_ << "mtllib " << fileName << std::endl;
}

void ObjWriter::Material(const std::string & name)
{
    stream_ << "mtl " << name << std::endl;
}

void ObjWriter::Object(const std::string& name)
{

    stream_ << "o " << name << std::endl;
}

void ObjWriter::Group(const std::string& name)
{
    stream_ << "g " << name << std::endl;
}

int ObjWriter::Vertex(float x, float y, float z)
{
    stream_ << "v " << x << ' ' << y << ' ' << z << std::endl;
    return vertexIndex_++;
}

void ObjWriter::BeginFace()
{
    stream_ << "f ";
}

void ObjWriter::EndFace()
{
    stream_ << std::endl;
}

void ObjWriter::Face(int index)
{
    stream_ << index << ' ';
}
