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

#include "stdafx.h"
#include <Urho3D/Urho3D.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Timer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Graphics/IndexBuffer.h>
#include <Urho3D/Math/BoundingBox.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <SDL/SDL_log.h>

#if _MSC_VER
#undef M_PI
#include <Urho3D/Math/MathDefs.h>
#endif

#include "Ocean.h"
#include "ComplexFFT.h"

#include <Urho3D/DebugNew.h>

//=============================================================================
//=============================================================================
#define GRAVITY            9.81f
#define FRAME_RATE_MS      33

//=============================================================================
//=============================================================================
float uniformRandomVariable()
{
	return (float)rand()/RAND_MAX;
}

complex gaussianRandomVariable()
{
	float x1, x2, w;
	do {
	    x1 = 2.f * uniformRandomVariable() - 1.f;
	    x2 = 2.f * uniformRandomVariable() - 1.f;
	    w = x1 * x1 + x2 * x2;
	} while ( w >= 1.f );
	w = sqrt((-2.f * log(w)) / w);
	return complex(x1 * w, x2 * w);
}

//=============================================================================
//=============================================================================
void Ocean::RegisterObject(Context *context)
{
    context->RegisterFactory<Ocean>();
}

Ocean::Ocean(Context *context)
    : Component(context)
    , pCOcean(NULL)
    , threadProcess_(NULL)
{
}

Ocean::~Ocean()
{
    if ( threadProcess_ )
    {
        delete threadProcess_;
        threadProcess_ = NULL;
    }

    if ( pCOcean )
    {
        delete pCOcean;
        pCOcean = NULL;
    }
}

void Ocean::InitOcean()
{
    N = 64;
    Nplus1 = N+1;

    MakeMesh(Nplus1, m_mesh);

    //pCOcean = new cOcean(64, 0.0005f, Vector2(32.0f, 32.0f),   64, false);
    //pCOcean = new cOcean(N,  0.004f,  Vector2( 3.0f,  0.6f),  800, false); // works ok
    //pCOcean = new cOcean(N,   4e-5f,  Vector2( 6.0f,  0.02f), 800, false); // works ok
    //pCOcean = new cOcean(N,   4e-6f,  Vector2( 6.0f,  6.0f),  800, false); // works ok
    //pCOcean = new cOcean(N,   4e-6f,  Vector2( 8.0f,  8.0f),  800, false); // works ok
    pCOcean = new cOcean(N,   4e-6f,  Vector2(1.0f, 12.0f),   800, false); // works ok

    // start thread
    elapsedFrameTimer_ = new Time(context_);
    threadProcess_ = new HelperThread<Ocean>(this, &Ocean::BackgroundProcess);
    threadProcess_->Start();

    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(Ocean, HandleUpdate));
}

void Ocean::SetProcessPending(bool bset)
{
    MutexLock lock(mutexPendingLock_);
    processPending_ = bset;
}

bool Ocean::IsProcessPending()
{
    MutexLock lock(mutexPendingLock_);
    return processPending_;
}

void Ocean::BackgroundProcess()
{
    // fixed at 30 fps
    if ( processTimer_.GetMSec(false) < FRAME_RATE_MS )
        return;

    if ( IsProcessPending() )
    {
        EvaluateWavesFFT();

        SetProcessPending(false);
    }
}

void Ocean::HandleUpdate(StringHash, VariantMap&)
{
    if ( !IsProcessPending() )
    {
        UpdateVertexBuffer();

        SetProcessPending(true);
    }
}

void Ocean::EvaluateWavesFFT()
{
    // process FFT
    float t = elapsedFrameTimer_->GetElapsedTime();// * 2.0f; // increase the wave change rate
    pCOcean->evaluateWavesFFT( t );

    // reset process timer
    processTimer_.Reset();
}

void Ocean::UpdateVertexBuffer()
{
    Geometry *pGeometry = m_pModelOcean->GetGeometry(0, 0);
    VertexBuffer *pVbuffer = pGeometry->GetVertexBuffer(0);

    unsigned uElementMask = pVbuffer->GetElementMask();
    unsigned vertexSize = pVbuffer->GetVertexSize();
    const unsigned char *pVertexData = (const unsigned char*)pVbuffer->Lock(0, pVbuffer->GetVertexCount());

    BoundingBox bbox;

    // get verts, normals, uv, etc.
    if ( pVertexData )
    {
        unsigned numVertices = pVbuffer->GetVertexCount();

        for ( unsigned i = 0; i < numVertices; ++i )
        {
            unsigned char *pDataAlign = (unsigned char *)(pVertexData + i * vertexSize);

            if ( uElementMask & MASK_POSITION )
            {
                Vector3 &vPos = *reinterpret_cast<Vector3*>( pDataAlign );
                pDataAlign += sizeof( Vector3 );

                Vector3 wave = Vector3( pCOcean->vertices[ i ].x, pCOcean->vertices[ i ].y, pCOcean->vertices[ i ].z );
                vPos = wave;

                // adj pos and scale
                wave = wave * node_->GetScale();
                wave += node_->GetPosition();
                m_mesh.vertices[ i ] = wave;

                bbox.Merge( wave );
            }

            if ( uElementMask & MASK_NORMAL )
            {
                Vector3 &vNorm = *reinterpret_cast<Vector3*>( pDataAlign );
                pDataAlign += sizeof( Vector3 );

                // normal list
                vNorm = Vector3( pCOcean->vertices[ i ].nx, pCOcean->vertices[ i ].ny, pCOcean->vertices[ i ].nz );
            }
        }

        //unlock
        pVbuffer->Unlock();
    }

    if ( (bbox.Size() - m_BoundingBox.Size()).Length() > 5.0f )
    {
        m_BoundingBox.Merge( bbox );
        m_pModelOcean->SetBoundingBox( m_BoundingBox );
    }
}

void Ocean::MakeMesh(int size, Mesh &mesh)
{
    mesh.vertices.Resize( size*size );
    mesh.texcoords.Resize( size*size );
    mesh.normals.Resize( size*size );
    int sizen_1 = size - 1;
    mesh.indices.Resize( 6*sizen_1*sizen_1 );
    BoundingBox bbox;

    for(int x = 0; x < size; x++)
    {
        for(int y = 0; y < size; y++)
        {
            Vector2 uv = Vector2( (float)x / (float)(size-1), (float)y / (float)(size-1) );
            Vector3 pos = Vector3((float)x, 0.0f, (float)y);
            Vector3 norm = Vector3(0.0f, 1.0f, 0.0f);

            mesh.texcoords[x+y*size] = uv;
            mesh.vertices[x+y*size] = pos;
            mesh.normals[x+y*size] = norm;

            m_BoundingBox.Merge( pos );
        }
    }

    // indeces are set for cOcean geometry = false -> set to triangles
    int num = 0;
    for(int x = 0; x < sizen_1; x++)
    {
        for(int y = 0; y < sizen_1; y++)
        {
            int index = x * size + y;
            mesh.indices[num++] = index;
            mesh.indices[num++] = index + size;
            mesh.indices[num++] = index + size + 1;

            mesh.indices[num++] = index;
            mesh.indices[num++] = index + size + 1;
            mesh.indices[num++] = index + 1;
        }
    }

    // new vertex buffer
    SharedPtr<VertexBuffer> vtxbuffer( new VertexBuffer( context_ ) );
    unsigned numVertices = mesh.vertices.Size();
    unsigned uElementMask = MASK_POSITION | MASK_NORMAL | MASK_TEXCOORD1;

    vtxbuffer->SetShadowed( true );
    vtxbuffer->SetSize( numVertices, uElementMask );
    unsigned vertexSize = vtxbuffer->GetVertexSize();
    unsigned char *pVertexData = (unsigned char*)vtxbuffer->Lock(0, vtxbuffer->GetVertexCount());

    if ( pVertexData )
    {
        for ( unsigned i = 0; i < numVertices; ++i )
        {
            unsigned char *pDataAlign = (unsigned char *)(pVertexData + i * vertexSize);

            if ( uElementMask & MASK_POSITION )
            {
                Vector3 &vPos = *reinterpret_cast<Vector3*>( pDataAlign );
                pDataAlign += sizeof( Vector3 );

                vPos = mesh.vertices[ i ];
            }

            if ( uElementMask & MASK_NORMAL )
            {
                Vector3 &vNorm = *reinterpret_cast<Vector3*>( pDataAlign );
                pDataAlign += sizeof( Vector3 );

                vNorm = mesh.normals[ i ];
            }

            if ( uElementMask & MASK_TEXCOORD1 )
            {
                Vector2 &vUV = *reinterpret_cast<Vector2*>( pDataAlign );
                pDataAlign += sizeof( Vector2 );

                vUV = mesh.texcoords[ i ];
            }
        }

        //unlock
        vtxbuffer->Unlock();
    }

    // new index buffer
    SharedPtr<IndexBuffer> idxbuffer( new IndexBuffer( context_ ) );
    unsigned numIndeces = mesh.indices.Size();

    idxbuffer->SetShadowed( true );
    idxbuffer->SetSize( numIndeces, false );

    const unsigned *pIndexData = (const unsigned *)idxbuffer->Lock( 0, idxbuffer->GetIndexCount() );
    unsigned short *pUShortData = (unsigned short *)pIndexData;

    if ( pUShortData )
    {
        for( unsigned i = 0; i < numIndeces; ++i )
        {
            pUShortData[ i ] = (unsigned short)mesh.indices[ i ];
        }

        idxbuffer->Unlock();
    }

    // new model
    m_pModelOcean = new Model(context_);

    // vtx buffer
    Vector<SharedPtr<VertexBuffer> > vtxBuffers;
    PODVector<unsigned> morphRangeStarts;
    PODVector<unsigned> morphRangeCounts;
    morphRangeStarts.Push( 0 );
    morphRangeCounts.Push( 0 );
    vtxBuffers.Push( vtxbuffer );

    // idx buffer
    Vector<SharedPtr<IndexBuffer> > idxBuffers;
    idxBuffers.Push( idxbuffer );

    // set model data
    m_pModelOcean->SetVertexBuffers( vtxBuffers, morphRangeStarts, morphRangeCounts );
    m_pModelOcean->SetIndexBuffers( idxBuffers );

    SharedPtr<Geometry> pGeometry;
    pGeometry = new Geometry(context_);
    pGeometry->SetNumVertexBuffers( 1 );
    pGeometry->SetVertexBuffer( 0, vtxbuffer );
    pGeometry->SetIndexBuffer( idxbuffer );
    pGeometry->SetDrawRange(TRIANGLE_LIST, 0, numIndeces);

    m_pModelOcean->SetNumGeometries( 1 );
    m_pModelOcean->SetNumGeometryLodLevels(0, 1);
    m_pModelOcean->SetGeometry(0, 0, pGeometry);
    m_pModelOcean->SetBoundingBox( m_BoundingBox );
    m_pModelOcean->SetGeometryCenter( 0, m_BoundingBox.Center() );

    SDL_Log( "ocean bounding box (%4.0f,%4.0f,%4.0f)\n",m_BoundingBox.Size().x_,m_BoundingBox.Size().y_,m_BoundingBox.Size().z_  );
}

void Ocean::DbgRender()
{
    DebugRenderer *dbg = GetScene()->GetComponent<DebugRenderer>();

    for ( unsigned i = 0; i < m_mesh.indices.Size(); i += 3 )
    {
        int i0 = m_mesh.indices[i ];
        int i1 = m_mesh.indices[i+1];
        int i2 = m_mesh.indices[i+2];

        Vector3 v0 = m_mesh.vertices[ i0 ];
        Vector3 v1 = m_mesh.vertices[ i1 ];
        Vector3 v2 = m_mesh.vertices[ i2 ];

        // only draw two sides of triangles
        dbg->AddLine( v0, v1,  Color::GREEN );
        dbg->AddLine( v1, v2,  Color::GREEN );
    }
}

//=============================================================================
//=============================================================================
cOcean::cOcean(const int N, const float A, const Vector2 w, const float length, const bool _geometry) :
	g(GRAVITY), geometry(_geometry), N(N), Nplus1(N+1), A(A), w(w), length(length),
	vertices(0), indices(0), h_tilde(0), h_tilde_slopex(0), h_tilde_slopez(0), h_tilde_dx(0), h_tilde_dz(0), fft(0)
{
	h_tilde        = new complex[N*N];
	h_tilde_slopex = new complex[N*N];
	h_tilde_slopez = new complex[N*N];
	h_tilde_dx     = new complex[N*N];
	h_tilde_dz     = new complex[N*N];
	fft            = new ComplexFFT(N);
	vertices       = new vertex_ocean[Nplus1*Nplus1];
	indices        = new unsigned int[Nplus1*Nplus1*10];

	int index;

	complex htilde0, htilde0mk_conj;
	for (int m_prime = 0; m_prime < Nplus1; m_prime++) {
		for (int n_prime = 0; n_prime < Nplus1; n_prime++) {
			index = m_prime * Nplus1 + n_prime;

			htilde0        = hTilde_0( n_prime,  m_prime);
			htilde0mk_conj = hTilde_0(-n_prime, -m_prime).conj();

			vertices[index].a  = htilde0.a;
			vertices[index].b  = htilde0.b;
			vertices[index]._a = htilde0mk_conj.a;
			vertices[index]._b = htilde0mk_conj.b;

			vertices[index].ox = vertices[index].x =  (n_prime - N / 2.0f) * length / N;
			vertices[index].oy = vertices[index].y =  0.0f;
			vertices[index].oz = vertices[index].z =  (m_prime - N / 2.0f) * length / N;

			vertices[index].nx = 0.0f;
			vertices[index].ny = 1.0f;
			vertices[index].nz = 0.0f;
		}
	}

    // always use triangles
    geometry = false;
	indices_count = 0;
	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			index = m_prime * Nplus1 + n_prime;

			if (geometry) {
				indices[indices_count++] = index;				// lines
				indices[indices_count++] = index + 1;
				indices[indices_count++] = index;
				indices[indices_count++] = index + Nplus1;
				indices[indices_count++] = index;
				indices[indices_count++] = index + Nplus1 + 1;
				if (n_prime == N - 1) {
					indices[indices_count++] = index + 1;
					indices[indices_count++] = index + Nplus1 + 1;
				}
				if (m_prime == N - 1) {
					indices[indices_count++] = index + Nplus1;
					indices[indices_count++] = index + Nplus1 + 1;
				}
			} else {
				indices[indices_count++] = index;				// two triangles
				indices[indices_count++] = index + Nplus1;
				indices[indices_count++] = index + Nplus1 + 1;
				indices[indices_count++] = index;
				indices[indices_count++] = index + Nplus1 + 1;
				indices[indices_count++] = index + 1;
			}
		}
	}
}

cOcean::~cOcean() {
	if (h_tilde)		delete [] h_tilde;
	if (h_tilde_slopex)	delete [] h_tilde_slopex;
	if (h_tilde_slopez)	delete [] h_tilde_slopez;
	if (h_tilde_dx)		delete [] h_tilde_dx;
	if (h_tilde_dz)		delete [] h_tilde_dz;
	if (fft)		    delete fft;
	if (vertices)		delete [] vertices;
	if (indices)		delete [] indices;
}

void cOcean::release() {
}

float cOcean::dispersion(int n_prime, int m_prime) {
	float w_0 = 2.0f * M_PI / 200.0f;
	float kx = M_PI * (2.0f * n_prime - N) / length;
	float kz = M_PI * (2.0f * m_prime - N) / length;
	return floor(sqrt(g * sqrt(kx * kx + kz * kz)) / w_0) * w_0;
}

float cOcean::phillips(int n_prime, int m_prime) {
	Vector2 k(M_PI * (2.0f * n_prime - N) / length, M_PI * (2 * m_prime - N) / length);
	float k_length  = k.Length();

	if (k_length < 0.000001f) return 0.0f;

	float k_length2 = k_length  * k_length;
	float k_length4 = k_length2 * k_length2;

	float k_dot_w   = k.Normalized().DotProduct( w.Normalized() );
    float k3_dot_w2 = k_dot_w * k_dot_w * k_dot_w;
	float k_dot_w2  = k3_dot_w2 * k3_dot_w2;

	float w_length  = w.Length();
	float L         = w_length * w_length / g;
	float L2        = L * L;

	float damping   = 0.001f;
	float l2        = L2 * damping * damping;

	return A * exp(-1.0f / (k_length2 * L2)) / k_length4 * k_dot_w2 * exp(-k_length2 * l2);
}

complex cOcean::hTilde_0(int n_prime, int m_prime) {
	complex r = gaussianRandomVariable();
	return r * sqrt(phillips(n_prime, m_prime) / 2.0f);
}

complex cOcean::hTilde(float t, int n_prime, int m_prime) {
	int index = m_prime * Nplus1 + n_prime;

	complex htilde0(vertices[index].a,  vertices[index].b);
	complex htilde0mkconj(vertices[index]._a, vertices[index]._b);

	float omegat = dispersion(n_prime, m_prime) * t;

	float cos_ = cos(omegat);
	float sin_ = sin(omegat);

	complex c0(cos_,  sin_);
	complex c1(cos_, -sin_);

	complex res = htilde0 * c0 + htilde0mkconj * c1;

	return htilde0 * c0 + htilde0mkconj*c1;
}

complex_vector_normal cOcean::h_D_and_n(Vector2      x, float t) {
	complex h(0.0f, 0.0f);
	Vector2      D(0.0f, 0.0f);
	Vector3 n(0.0f, 0.0f, 0.0f);

	complex c, res, htilde_c;
	Vector2      k;
	float kx, kz, k_length, k_dot_x;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		kz = 2.0f * M_PI * (m_prime - N / 2.0f) / length;
		for (int n_prime = 0; n_prime < N; n_prime++) {
			kx = 2.0f * M_PI * (n_prime - N / 2.0f) / length;
			k = Vector2     (kx, kz);

			k_length = k.Length();
			k_dot_x = k.DotProduct( x );

			c = complex(cos(k_dot_x), sin(k_dot_x));
			htilde_c = hTilde(t, n_prime, m_prime) * c;

			h = h + htilde_c;

			n = n + Vector3(-kx * htilde_c.b, 0.0f, -kz * htilde_c.b);

			if (k_length < 0.000001f) continue;
			D = D + Vector2     (kx / k_length * htilde_c.b, kz / k_length * htilde_c.b);
		}
	}

	n = (Vector3(0.0f, 1.0f, 0.0f) - n).Normalized();

	complex_vector_normal cvn;
	cvn.h = h;
	cvn.D = D;
	cvn.n = n;
	return cvn;
}

// this shows how slow the simulation is w/o FFT
void cOcean::evaluateWaves(float t) {
	float lambda = -1.0;
	int index;
	Vector2      x;
	Vector2      d;
	complex_vector_normal h_d_and_n;
	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			index = m_prime * Nplus1 + n_prime;

			x = Vector2     (vertices[index].x, vertices[index].z);

			h_d_and_n = h_D_and_n(x, t);

			vertices[index].y = h_d_and_n.h.a;

			vertices[index].x = vertices[index].ox + lambda*h_d_and_n.D.x_;
			vertices[index].z = vertices[index].oz + lambda*h_d_and_n.D.y_;

			vertices[index].nx = h_d_and_n.n.x_;
			vertices[index].ny = h_d_and_n.n.y_;
			vertices[index].nz = h_d_and_n.n.z_;

			if (n_prime == 0 && m_prime == 0) {
				vertices[index + N + Nplus1 * N].y = h_d_and_n.h.a;

				vertices[index + N + Nplus1 * N].x = vertices[index + N + Nplus1 * N].ox + lambda*h_d_and_n.D.x_;
				vertices[index + N + Nplus1 * N].z = vertices[index + N + Nplus1 * N].oz + lambda*h_d_and_n.D.y_;

				vertices[index + N + Nplus1 * N].nx = h_d_and_n.n.x_;
				vertices[index + N + Nplus1 * N].ny = h_d_and_n.n.y_;
				vertices[index + N + Nplus1 * N].nz = h_d_and_n.n.z_;
			}
			if (n_prime == 0) {
				vertices[index + N].y = h_d_and_n.h.a;

				vertices[index + N].x = vertices[index + N].ox + lambda*h_d_and_n.D.x_;
				vertices[index + N].z = vertices[index + N].oz + lambda*h_d_and_n.D.y_;

				vertices[index + N].nx = h_d_and_n.n.x_;
				vertices[index + N].ny = h_d_and_n.n.y_;
				vertices[index + N].nz = h_d_and_n.n.z_;
			}
			if (m_prime == 0) {
				vertices[index + Nplus1 * N].y = h_d_and_n.h.a;

				vertices[index + Nplus1 * N].x = vertices[index + Nplus1 * N].ox + lambda*h_d_and_n.D.x_;
				vertices[index + Nplus1 * N].z = vertices[index + Nplus1 * N].oz + lambda*h_d_and_n.D.y_;

				vertices[index + Nplus1 * N].nx = h_d_and_n.n.x_;
				vertices[index + Nplus1 * N].ny = h_d_and_n.n.y_;
				vertices[index + Nplus1 * N].nz = h_d_and_n.n.z_;
			}
		}
	}
}

void cOcean::evaluateWavesFFT(float t)
{
	float kx, kz, len, lambda = -1.0f;
	int index, index1;

	for (int m_prime = 0; m_prime < N; m_prime++) {
		kz = M_PI * (2.0f * m_prime - N) / length;
		for (int n_prime = 0; n_prime < N; n_prime++) {
			kx = M_PI*(2.0f * n_prime - N) / length;
			len = sqrt(kx * kx + kz * kz);
			index = m_prime * N + n_prime;

			h_tilde[index] = hTilde(t, n_prime, m_prime);
			h_tilde_slopex[index] = h_tilde[index] * complex(0, kx);
			h_tilde_slopez[index] = h_tilde[index] * complex(0, kz);
			if (len < 0.000001f) {
				h_tilde_dx[index]     = complex(0.0f, 0.0f);
				h_tilde_dz[index]     = complex(0.0f, 0.0f);
			} else {
				h_tilde_dx[index]     = h_tilde[index] * complex(0, -kx/len);
				h_tilde_dz[index]     = h_tilde[index] * complex(0, -kz/len);
			}
		}
	}

	for (int m_prime = 0; m_prime < N; m_prime++) {
		fft->fft(h_tilde, h_tilde, 1, m_prime * N);
		fft->fft(h_tilde_slopex, h_tilde_slopex, 1, m_prime * N);
		fft->fft(h_tilde_slopez, h_tilde_slopez, 1, m_prime * N);
		fft->fft(h_tilde_dx, h_tilde_dx, 1, m_prime * N);
		fft->fft(h_tilde_dz, h_tilde_dz, 1, m_prime * N);
	}
	for (int n_prime = 0; n_prime < N; n_prime++) {
		fft->fft(h_tilde, h_tilde, N, n_prime);
		fft->fft(h_tilde_slopex, h_tilde_slopex, N, n_prime);
		fft->fft(h_tilde_slopez, h_tilde_slopez, N, n_prime);
		fft->fft(h_tilde_dx, h_tilde_dx, N, n_prime);
		fft->fft(h_tilde_dz, h_tilde_dz, N, n_prime);
	}

	float sign;
	float signs[] = { 1.0f, -1.0f };
	Vector3 n;
	for (int m_prime = 0; m_prime < N; m_prime++) {
		for (int n_prime = 0; n_prime < N; n_prime++) {
			index  = m_prime * N + n_prime;		// index into h_tilde..
			index1 = m_prime * Nplus1 + n_prime;	// index into vertices

			sign = signs[(n_prime + m_prime) & 1];

			h_tilde[index]     = h_tilde[index] * sign;

			// height
			vertices[index1].y = h_tilde[index].a;

			// displacement
			h_tilde_dx[index] = h_tilde_dx[index] * sign;
			h_tilde_dz[index] = h_tilde_dz[index] * sign;
			vertices[index1].x = vertices[index1].ox + h_tilde_dx[index].a * lambda;
			vertices[index1].z = vertices[index1].oz + h_tilde_dz[index].a * lambda;

			// normal
			h_tilde_slopex[index] = h_tilde_slopex[index] * sign;
			h_tilde_slopez[index] = h_tilde_slopez[index] * sign;
			n = Vector3(0.0f - h_tilde_slopex[index].a, 1.0f, 0.0f - h_tilde_slopez[index].a).Normalized();
			vertices[index1].nx =  n.x_;
			vertices[index1].ny =  n.y_;
			vertices[index1].nz =  n.z_;

			// for tiling
			if (n_prime == 0 && m_prime == 0) {
				vertices[index1 + N + Nplus1 * N].y = h_tilde[index].a;

				vertices[index1 + N + Nplus1 * N].x = vertices[index1 + N + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + N + Nplus1 * N].z = vertices[index1 + N + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + N + Nplus1 * N].nx =  n.x_;
				vertices[index1 + N + Nplus1 * N].ny =  n.y_;
				vertices[index1 + N + Nplus1 * N].nz =  n.z_;
			}
			if (n_prime == 0) {
				vertices[index1 + N].y = h_tilde[index].a;

				vertices[index1 + N].x = vertices[index1 + N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + N].z = vertices[index1 + N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + N].nx =  n.x_;
				vertices[index1 + N].ny =  n.y_;
				vertices[index1 + N].nz =  n.z_;
			}
			if (m_prime == 0) {
				vertices[index1 + Nplus1 * N].y = h_tilde[index].a;

				vertices[index1 + Nplus1 * N].x = vertices[index1 + Nplus1 * N].ox + h_tilde_dx[index].a * lambda;
				vertices[index1 + Nplus1 * N].z = vertices[index1 + Nplus1 * N].oz + h_tilde_dz[index].a * lambda;

				vertices[index1 + Nplus1 * N].nx =  n.x_;
				vertices[index1 + Nplus1 * N].ny =  n.y_;
				vertices[index1 + Nplus1 * N].nz =  n.z_;
			}
		}
	}
}


