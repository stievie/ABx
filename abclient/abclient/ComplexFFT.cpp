#include "stdafx.h"

//=============================================================================
//=============================================================================

#include <Urho3D/Urho3D.h>
#include <Urho3D/Math/MathDefs.h>
#include <Urho3D/Math/Vector4.h>

#include "ComplexFFT.h"

#include <SDL/SDL_log.h>
#include <Urho3D/DebugNew.h>


//=============================================================================
//=============================================================================
unsigned int complex::additions = 0;
unsigned int complex::multiplications = 0;

complex::complex() : a(0.0f), b(0.0f) { }
complex::complex(float a, float b) : a(a), b(b) { }
complex complex::conj() { return complex(this->a, -this->b); }

complex complex::operator*(const complex& c) const {
	complex::multiplications++;
	return complex(this->a*c.a - this->b*c.b, this->a*c.b + this->b*c.a);
}

complex complex::operator+(const complex& c) const {
	complex::additions++;
	return complex(this->a + c.a, this->b + c.b);
}

complex complex::operator-(const complex& c) const {
	complex::additions++;
	return complex(this->a - c.a, this->b - c.b);
}

complex complex::operator-() const {
	return complex(-this->a, -this->b);
}

complex complex::operator*(const float c) const {
	return complex(this->a*c, this->b*c);
}

complex& complex::operator=(const complex& c) {
	this->a = c.a; this->b = c.b;
	return *this;
}

void complex::reset() {
	complex::additions = 0;
	complex::multiplications = 0;
}


ComplexFFT::ComplexFFT(unsigned int N) : N(N), reversed(0), T(0), pi2(2 * M_PI) {
	c[0] = c[1] = 0;

	log_2_N = static_cast<unsigned>(log(N) / log(2));

	reversed = new unsigned int[N];		// prep bit reversals
	for (unsigned i = 0; i < N; i++) reversed[i] = reverse(i);

	int pow2 = 1;
	T = new complex*[log_2_N];		// prep T
	for (unsigned i = 0; i < log_2_N; i++) {
		T[i] = new complex[pow2];
		for (int j = 0; j < pow2; j++) T[i][j] = t(j, pow2 * 2);
		pow2 *= 2;
	}

	c[0] = new complex[N];
	c[1] = new complex[N];
	which = 0;
}

ComplexFFT::~ComplexFFT() {
	if (c[0]) delete [] c[0];
	if (c[1]) delete [] c[1];
	if (T) {
		for (unsigned i = 0; i < log_2_N; i++) if (T[i]) delete [] T[i];
		delete [] T;
	}
	if (reversed) delete [] reversed;
}

unsigned int ComplexFFT::reverse(unsigned int i) {
	unsigned int res = 0;
	for (unsigned j = 0; j < log_2_N; j++) {
		res = (res << 1) + (i & 1);
		i >>= 1;
	}
	return res;
}

complex ComplexFFT::t(unsigned int x, unsigned int _N) {
	return complex(cos(pi2 * x / _N), sin(pi2 * x / _N));
}

void ComplexFFT::fft(complex* input, complex* output, int stride, int offset) {
	for (unsigned i = 0; i < N; i++) c[which][i] = input[reversed[i] * stride + offset];

	int loops       = N>>1;
	int size        = 1<<1;
	int size_over_2 = 1;
	int w_          = 0;
	for (unsigned i = 1; i <= log_2_N; i++) {
		which ^= 1;
		for (int j = 0; j < loops; j++) {
			for (int k = 0; k < size_over_2; k++) {
				c[which][size * j + k] =  c[which^1][size * j + k] +
							  c[which^1][size * j + size_over_2 + k] * T[w_][k];
			}

			for (int k = size_over_2; k < size; k++) {
				c[which][size * j + k] =  c[which^1][size * j - size_over_2 + k] -
							  c[which^1][size * j + k] * T[w_][k - size_over_2];
			}
		}
		loops       >>= 1;
		size        <<= 1;
		size_over_2 <<= 1;
		w_++;
	}

	for (unsigned i = 0; i < N; i++) output[i * stride + offset] = c[which][i];
}

