//=============================================================================
//=============================================================================

#pragma once

//#include <Urho3D/Urho3D.h>
//#include <Urho3D/Container/Vector.h>
#include <Urho3D/Container/ArrayPtr.h>

using namespace Urho3D;

//=============================================================================
//=============================================================================
class complex {
  private:
  protected:
  public:
    float a, b;
    static unsigned int additions, multiplications;
    complex();
    complex(float a, float b);
    complex conj();
    complex operator*(const complex& c) const;
    complex operator+(const complex& c) const;
    complex operator-(const complex& c) const;
    complex operator-() const;
    complex operator*(const float c) const;
    complex& operator=(const complex& c);
    static void reset();
};

class ComplexFFT {
  private:
	unsigned int N, which;
	unsigned int log_2_N;
	float pi2;
	unsigned int *reversed;
	complex **T;
	complex *c[2];
  protected:
  public:
	ComplexFFT(unsigned int N);
	~ComplexFFT();
	unsigned int reverse(unsigned int i);
	complex t(unsigned int x, unsigned int N);
	void fft(complex* input, complex* output, int stride, int offset);
};

