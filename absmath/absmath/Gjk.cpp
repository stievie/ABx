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

#include "Gjk.h"

namespace Math {

bool Gjk::CheckOneFaceAC(const Vector3& abc, const Vector3& ac, const Vector3& ao)
{
    if (abc.CrossProduct(ac).DotProduct(ao) > 0.0f)
    {
        //origin is in the region of edge ac
        b = -ao;
        v = TripleProduct(ac, ao);
        n = 2;
        return false;
    }
    return true;
}

bool Gjk::CheckOneFaceAB(const Vector3& abc, const Vector3& ab, const Vector3& ao)
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

bool Gjk::CheckTwoFaces(Vector3& abc, Vector3& acd, Vector3& ac, Vector3& ab, Vector3& ad, const Vector3& ao)
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

bool Gjk::Update(const Vector3& a)
{
    if (n == 2)
    {
        // Handling triangle
        const Vector3 ao = -a;
        const Vector3 ab = b - a;
        const Vector3 ac = c - a;

        // Normal of triangle ABC
        const Vector3 abc = ab.CrossProduct(ac);

        // Plane test on edge ab
        const Vector3 abp = ab.CrossProduct(abc);       // direction vector pointing inside triangle abc from ab
        if (abp.DotProduct(ao) > 0.0f)
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
        const Vector3 acp = abc.CrossProduct(ac);
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
        const Vector3 ao = -a;
        Vector3 ab = b - a;
        Vector3 ac = c - a;
        Vector3 ad = d - a;

        Vector3 abc = ab.CrossProduct(ac);
        Vector3 acd = ac.CrossProduct(ad);
        const Vector3 adb = ad.CrossProduct(ab);

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
            return true;
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
        default:
            return true;
        }
    }

    return true;
}

bool Gjk::Intersects(const Shape& shape1, const Shape& shape2)
{
    v = Vector3::UnitX;
    n = 0;

    c = Support(shape1, shape2, v);

    if (c.DotProduct(v) < 0.0f)
        return false;
    v = -c;

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

}
