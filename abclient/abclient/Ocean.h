//=============================================================================
// Copyright (c) 2016 Lumak
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
//=============================================================================

#pragma once

#include <Urho3D/Container/Vector.h>

#include "HelperThread.h"
#include "ComplexFFT.h"

namespace Urho3D
{
class Material;
class Model;
class Timer;
}

using namespace Urho3D;

//=============================================================================
//=============================================================================
struct vertex_ocean
{
	float   x,   y,   z; // vertex
	float  nx,  ny,  nz; // normal
	float   a,   b,   c; // htilde0
	float  _a,  _b,  _c; // htilde0mk conjugate
	float  ox,  oy,  oz; // original position
};

// structure used with discrete fourier transform
struct complex_vector_normal
{
	complex h; // wave height
	Vector2 D; // displacement
	Vector3 n; // normal
};


//=============================================================================
//=============================================================================
class cOcean {
  private:
	bool geometry;			// flag to render geometry or surface

	float g;				// gravity constant
	int N, Nplus1;			// dimension -- N should be a power of 2
	float A;				// phillips spectrum parameter -- affects heights of waves
	Vector2      w;			// wind parameter
	float length;			// length parameter
	complex *h_tilde,		// for fast fourier transform
		*h_tilde_slopex, *h_tilde_slopez,
		*h_tilde_dx, *h_tilde_dz;
	ComplexFFT *fft;				// fast fourier transform

public:
	vertex_ocean *vertices;			// vertices for vertex buffer object
	unsigned int *indices;			// indicies for vertex buffer object
	unsigned int indices_count;		// number of indices to render
	//GLuint vbo_vertices, vbo_indices;	// vertex buffer objects

	//GLuint glProgram, glShaderV, glShaderF;	// shaders
	//GLint vertex, normal, texture, light_position, projection, view, model;	// attributes and uniforms

public:
	cOcean(const int N, const float A, const Vector2      w, const float length, bool geometry);
	~cOcean();
	void release();

	float dispersion(int n_prime, int m_prime);		// deep water
	float phillips(int n_prime, int m_prime);		// phillips spectrum
	complex hTilde_0(int n_prime, int m_prime);
	complex hTilde(float t, int n_prime, int m_prime);
	complex_vector_normal h_D_and_n(Vector2      x, float t);
	void evaluateWaves(float t);
	void evaluateWavesFFT(float t);
	//void render(float t, glm::vec3 light_pos, glm::mat4 Projection, glm::mat4 View, glm::mat4 Model, bool use_fft);
};

//=============================================================================
//=============================================================================
class Ocean : public Component
{
    URHO3D_OBJECT(Ocean, Component);

    struct Mesh
    {
        PODVector<Vector3> vertices;
        PODVector<Vector2> texcoords;
        PODVector<Vector3> normals;
        PODVector<int>     indices;
    };

public:
    static void RegisterObject(Context *context);

    Ocean(Context *context);
    ~Ocean();

    void InitOcean();

    Model* GetOceanModel() const        { return m_pModelOcean; }
    BoundingBox GetBoundingBox() const  { return m_BoundingBox; }

    void DbgRender();

protected:
    // fft
    void EvaluateWavesFFT();
    void UpdateVertexBuffer();
    void MakeMesh(int size, Mesh &mesh);

    // threading
    void SetProcessPending(bool bset);
    bool IsProcessPending();
    void BackgroundProcess();

    void HandleUpdate(StringHash eventType, VariantMap& eventData);

protected:
    // ocean
    cOcean *pCOcean;
    int     N;
    int     Nplus1;

    Mesh             m_mesh;
    SharedPtr<Model> m_pModelOcean;
    BoundingBox      m_BoundingBox;

    // background thread
    HelperThread<Ocean> *threadProcess_;
    Mutex               mutexPendingLock_;
    bool                processPending_;
    SharedPtr<Time>     elapsedFrameTimer_;
    Timer               processTimer_;
};



