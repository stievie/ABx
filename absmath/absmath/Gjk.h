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
