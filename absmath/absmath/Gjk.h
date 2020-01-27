/**
 * Copyright 2017-2020 Stefan Ascher
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

#pragma once

#include "Shape.h"

namespace Math {

/// https://github.com/xuzebin/gjk
class Gjk
{
private:
    Vector3 v;
    Vector3 b, c, d;
    /// Simplex size
    unsigned n;
    static constexpr int MAX_ITERATIONS = 30;
    static Vector3 TripleProduct(const Vector3& ab, const Vector3& _c)
    {
        return ab.CrossProduct(_c).CrossProduct(ab);
    }
    static Vector3 Support(const Shape& shape1, const Shape& shape2, const Vector3& _v)
    {
        const Vector3 p1 = shape1.GetFarsetPointInDirection(_v);
        const Vector3 p2 = shape2.GetFarsetPointInDirection(-_v);  //negate v
        return p1 - p2;
    }
    bool CheckOneFaceAC(const Vector3& abc, const Vector3& ac, const Vector3& ao);
    bool CheckOneFaceAB(const Vector3& abc, const Vector3& ab, const Vector3& ao);
    bool CheckTwoFaces(Vector3& abc, Vector3& acd, Vector3& ac, Vector3& ab, Vector3& ad, const Vector3& ao);
    bool Update(const Vector3& a);
public:
    Gjk() :
        n(0)
    { }
    ~Gjk() = default;

    static inline bool StaticIntersects(const Shape& shape1, const Shape& shape2)
    {
        Gjk gjk;
        return gjk.Intersects(shape1, shape2);
    }

    bool Intersects(const Shape& shape1, const Shape& shape2);
};

}
