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

void ObjWriter::Vertex(float x, float y, float z)
{
    stream_ << "v " << x << ' ' << y << ' ' << z << std::endl;
}

void ObjWriter::Normal(float x, float y, float z)
{
    if (normals_)
        stream_ << "vn " << x << ' ' << y << ' ' << z << std::endl;
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
    stream_ << index;
    if (normals_)
        stream_ << "//" << index;
    stream_ << ' ';
}
