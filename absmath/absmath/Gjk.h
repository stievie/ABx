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
    Vector3 TripleProduct(const Vector3& ab, const Vector3& _c) const
    {
        return ab.CrossProduct(_c).CrossProduct(ab);
    }
    bool CheckOneFaceAC(const Vector3& abc, const Vector3& ac, const Vector3& ao)
    {
        if (abc.CrossProduct(ac).DotProduct(ao) > 0.0f)
        {
            //origin is in the region of edge ac
            b = -ao;
            v = TripleProduct(ac, ao);
            n = 2;
            return false;;
        }
        return true;
    }
    bool CheckOneFaceAB(const Vector3& abc, const Vector3& ab, const Vector3& ao)
    {
        if (ab.CrossProduct(abc).DotProduct(ao) > 0.0f)
        {
            c = b;
            b = -ao;
            v = TripleProduct(ab, ao);
            n = 2;
            return false;
        }
        return true;
    }
    bool CheckTwoFaces(Vector3& abc, Vector3& acd, Vector3& ac, Vector3& ab, Vector3& ad, const Vector3& ao)
    {
        if (abc.CrossProduct(ac).DotProduct(ao) > 0.0f)
        {
            b = c;
            c = d;
            ab = ac;
            ac = ad;

            abc = acd;
            return false;
        }
        return true;
    }
    bool Update(const Vector3& a)
    {
        if (n == 2)
        {
            // Handling triangle
            Vector3 ao = -a;
            Vector3 ab = b - a;
            Vector3 ac = c - a;

            // Normal of triangle ABC
            Vector3 abc = ab.CrossProduct(ac);

            // Plane test on edge ab
            Vector3 abp = ab.CrossProduct(abc);       // direction vector pointing inside triangle abc from ab
            if (ab.DotProduct(ao) > 0.0f)
            {
                // origin lies outside the triangle abc, near the edge ab
                c = b;
                b = a;
                v = TripleProduct(ab, ao);
                return false;
            }

            // plane test on edge ac

            // direction vector pointing inside triangle abc from ac
            // note that different than abp, the result of acp is abc cross ac, while abp is ab cross abc.
            // The order does matter. Based on the right-handed rule, we want the vector pointing inside the triangle.
            Vector3 acp = abc.CrossProduct(ac);
            if (acp.DotProduct(ao) > 0.0f)
            {
                // origin lies outside the triangle abc, near the edge ac
                b = a;
                v = TripleProduct(ac, ao);
                return false;
            }

            // Now the origin is within the triangle abc, either above or below it.
            if (abc.DotProduct(ao) > 0.0f)
            {
                // origin is above the triangle
                d = c;
                c = b;
                b = a;
                v = -abc;
            }
            else
            {
                // origin is below the triangle
                d = b;
                b = a;
                v = -abc;
            }

            n = 3;
            return false;
        }

        if (n == 3)
        {
            Vector3 ao = -a;
            Vector3 ab = b - a;
            Vector3 ac = c - a;
            Vector3 ad = d - a;

            Vector3 abc = ab.CrossProduct(ac);
            Vector3 acd = ac.CrossProduct(ad);
            Vector3 adb = ad.CrossProduct(ab);

            Vector3 tmp;

            const int over_abc = 0x1;
            const int over_acd = 0x2;
            const int over_adb = 0x4;

            int planeTest =
                (abc.DotProduct(ao) > 0.0f ? over_abc : 0) |
                (acd.DotProduct(ao) > 0.0f ? over_acd : 0) |
                (adb.DotProduct(ao) > 0.0f ? over_adb : 0);

            switch (planeTest)
            {
            case 0:
                // inside the tetrahedron
                return false;
            case over_abc:
                if (!CheckOneFaceAC(abc, ac, ao))
                    //in the region of AC
                    return false;
                if (!CheckOneFaceAB(abc, ab, ao))
                    //in the region of AB
                    return false;
                //otherwise, in the region of ABC
                d = c;
                c = b;
                b = a;
                v = abc;
                n = 3;
                return false;
            case over_acd:
                //rotate acd to abc, and perform the same procedure
                b = c;
                c = d;

                ab = ac;
                ac = acd;

                abc = acd;

                if (!CheckOneFaceAC(abc, ac, ao))
                    //in the region of AC (actually is ad)
                    return false;
                if (!CheckOneFaceAB(abc, ab, ao))
                    //in the region of AB (actually is ac)
                    return false;

                //otherwise, in the region of "ABC" (which is actually acd)
                d = c;
                c = b;
                b = a;
                v = abc;
                n = 3;
                return false;
            case over_adb:
                //rotate adb to abc, and perform the same procedure
                c = b;
                b = d;

                ac = ab;
                ab = ad;

                abc = adb;
                if (!CheckOneFaceAC(abc, ac, ao))
                    return false;
                if (!CheckOneFaceAB(abc, ab, ao))
                    return false;
                //otherwise, in the region of "ABC" (which is actually acd)
                d = c;
                c = b;
                b = a;
                v = abc;
                n = 3;
                return false;
            case over_abc | over_acd:
                if (!CheckTwoFaces(abc, acd, ac, ab, ad, ao))
                {
                    if (!CheckOneFaceAC(abc, ac, ao))
                        return false;
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    //otherwise, in the region of "ABC" (which is actually acd)
                    d = c;
                    c = b;;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
                else
                {
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
            case over_acd | over_adb:
                //rotate ACD, ADB into ABC, ACD
                tmp = b;
                b = c;
                c = d;
                d = tmp;

                tmp = a;
                ab = ac;
                ac = ad;
                ad = tmp;

                abc = acd;
                acd = adb;

                if (!CheckTwoFaces(abc, acd, ac, ab, ad, ao))
                {
                    if (!CheckOneFaceAC(abc, ac, ao))
                        return false;
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    //otherwise, in the region of "ABC" (which is actually acd)
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
                else
                {
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
            case over_adb | over_abc:
                //rotate ADB, ABC into ABC, ACD
                tmp = c;
                c = b;
                b = d;
                d = tmp;

                tmp = ac;
                ac = ab;
                ab = ad;
                ad = tmp;

                acd = abc;
                abc = adb;

                if (!CheckTwoFaces(abc, acd, ac, ab, ad, ao))
                {
                    if (!CheckOneFaceAC(abc, ac, ao))
                        return false;
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    //otherwise, in the region of "ABC" (which is actually acd)
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
                else
                {
                    if (!CheckOneFaceAB(abc, ab, ao))
                        return false;
                    d = c;
                    c = b;
                    b = a;
                    v = abc;
                    n = 3;
                    return false;
                }
            }
        }

        return true;
    }
public:
    Gjk() = default;
    ~Gjk() = default;

    static inline bool StaticIntersects(const Shape& shape1, const Shape& shape2)
    {
        Gjk gjk;
        return gjk.Intersects(shape1, shape2);
    }

    bool Intersects(const Shape& shape1, const Shape& shape2)
    {
        v = Vector3::Zero;
        n = 0;

        c = Support(shape1, shape2, v);

        if (c.DotProduct(c) < 0.0f)
            return false;
        c = -c;

        b = Support(shape1, shape2, v);
        if (b.DotProduct(v) < 0.0f)
            return false;

        v = TripleProduct(c - b, -b);
        n = 2;

        for (unsigned i = 0; i < MAX_ITERATIONS; ++i)
        {
            const Vector3 a = Support(shape1, shape2, v);
            if (a.DotProduct(v) < 0.0f)
                // No intersection
                return false;

            if (Update(a))
                return true;
        }
        return true;
    }

    Vector3 Support(const Shape& shape1, const Shape& shape2, const Vector3& _v) const
    {
        const Vector3 p1 = shape1.GetFarsetPointInDirection(_v);
        const Vector3 p2 = shape2.GetFarsetPointInDirection(-_v);  //negate v
        return p1 - p2;
    }
};

}
