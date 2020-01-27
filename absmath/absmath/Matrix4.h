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

#include "Vector4.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "MathConfig.h"

namespace Math {

class Matrix4
{
public:
    enum Index
    {
        Index00 = 0,
        Index10,
        Index20,
        Index30,
        Index01,
        Index11,
        Index21,
        Index31,
        Index02,
        Index12,
        Index22,
        Index32,
        Index03,
        Index13,
        Index23,
        Index33,
    };
public:
    constexpr Matrix4() noexcept :
        m_{
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }
    { }
    constexpr Matrix4(
        float v00, float v01, float v02, float v03,                             // Row 1
        float v10, float v11, float v12, float v13,                             // Row 2
        float v20, float v21, float v22, float v23,                             // Row 3
        float v30, float v31, float v32, float v33                              // Row 4
    ) noexcept :
        m_{
            v00, v10, v20, v30,
            v01, v11, v21, v31,
            v02, v12, v22, v32,
            v03, v13, v23, v33
        }
    { }
    Matrix4(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3) noexcept;
#if defined(HAVE_DIRECTX_MATH)
    Matrix4(const XMath::XMMATRIX& matrix) noexcept;
#endif

#if defined(HAVE_DIRECTX_MATH)
    operator XMath::XMMATRIX() const
    {
        return XMath::XMMatrixSet(
            m_[0], m_[4], m_[8], m_[12],
            m_[1], m_[5], m_[9], m_[13],
            m_[2], m_[6], m_[10], m_[14],
            m_[3], m_[7], m_[11], m_[15]
        );
    }
#endif

    const Vector3 operator *(const Vector3& rhs) const;
    const Vector4 operator *(const Vector4& rhs) const;
    const Matrix4 operator *(const Matrix4& rhs) const;
    const Matrix4 operator *(float rhs) const
    {
        return Matrix4(
            m_[Index00] * rhs,
            m_[Index01] * rhs,
            m_[Index02] * rhs,
            m_[Index03] * rhs,
            m_[Index10] * rhs,
            m_[Index11] * rhs,
            m_[Index12] * rhs,
            m_[Index13] * rhs,
            m_[Index20] * rhs,
            m_[Index21] * rhs,
            m_[Index22] * rhs,
            m_[Index23] * rhs,
            m_[Index30] * rhs,
            m_[Index31] * rhs,
            m_[Index32] * rhs,
            m_[Index33] * rhs
        );
    }

    Matrix4 operator +(const Matrix4& rhs) const
    {
        return Matrix4(
            m_[Index00] + rhs.m_[Index00],
            m_[Index01] + rhs.m_[Index01],
            m_[Index02] + rhs.m_[Index02],
            m_[Index03] + rhs.m_[Index03],
            m_[Index10] + rhs.m_[Index10],
            m_[Index11] + rhs.m_[Index11],
            m_[Index12] + rhs.m_[Index12],
            m_[Index13] + rhs.m_[Index13],
            m_[Index20] + rhs.m_[Index20],
            m_[Index21] + rhs.m_[Index21],
            m_[Index22] + rhs.m_[Index22],
            m_[Index23] + rhs.m_[Index23],
            m_[Index30] + rhs.m_[Index30],
            m_[Index31] + rhs.m_[Index31],
            m_[Index32] + rhs.m_[Index32],
            m_[Index33] + rhs.m_[Index33]
        );
    }
    Matrix4 operator -(const Matrix4& rhs) const
    {
        return Matrix4(
            m_[Index00] - rhs.m_[Index00],
            m_[Index01] - rhs.m_[Index01],
            m_[Index02] - rhs.m_[Index02],
            m_[Index03] - rhs.m_[Index03],
            m_[Index10] - rhs.m_[Index10],
            m_[Index11] - rhs.m_[Index11],
            m_[Index12] - rhs.m_[Index12],
            m_[Index13] - rhs.m_[Index13],
            m_[Index20] - rhs.m_[Index20],
            m_[Index21] - rhs.m_[Index21],
            m_[Index22] - rhs.m_[Index22],
            m_[Index23] - rhs.m_[Index23],
            m_[Index30] - rhs.m_[Index30],
            m_[Index31] - rhs.m_[Index31],
            m_[Index32] - rhs.m_[Index32],
            m_[Index33] - rhs.m_[Index33]
        );
    }

    Matrix4& Translate(const Vector3& v);
    Matrix4& Scale(const Vector3& v);

    Matrix4& RotateX(float ang);
    Matrix4& RotateY(float ang);
    Matrix4& RotateZ(float ang);
    Matrix4& Rotate(const Vector3& axis, float ang);
    Matrix4& Rotate(const Vector4& axisAngle);

    /// Get rotation part
    Quaternion Rotation() const;
    /// Get translation part
    Vector3 Translation() const;
    /// Get scaling part
    Vector3 Scaling() const;
    void Decompose(Vector3* scale, Quaternion* rotation, Vector3* translation) const
    {
        XMath::XMVECTOR s, r, t;
        XMath::XMMatrixDecompose(&s, &r, &t, *this);
        scale->x_ = XMath::XMVectorGetX(s);
        scale->y_ = XMath::XMVectorGetY(s);
        scale->z_ = XMath::XMVectorGetZ(s);
        rotation->x_ = XMath::XMVectorGetX(r);
        rotation->y_ = XMath::XMVectorGetY(r);
        rotation->z_ = XMath::XMVectorGetZ(r);
        rotation->w_ = XMath::XMVectorGetW(r);
        translation->x_ = XMath::XMVectorGetX(t);
        translation->y_ = XMath::XMVectorGetY(t);
        translation->z_ = XMath::XMVectorGetZ(t);
    }

    Matrix4 Transpose() const;
    float Determinant() const;
    Matrix4 Inverse() const;
    const float* Data() const { return &m_[0]; }

    static Matrix4 FromFrustum(float left, float right, float bottom, float top, float _near, float _far);
    static Matrix4 FromPerspective(float fovy, float aspect, float _near, float _far);
    static Matrix4 FromOrtho(float left, float right, float bottom, float top, float _near, float _far);
    static Matrix4 FromOrtho(float width, float height, float _near, float _far);
    static Matrix4 FromLookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
    static Matrix4 FromQuaternion(const Quaternion& q);
    static Matrix4 FromAxisAngle(const Vector3& axis, float angle);
    static Matrix4 FromScale(const Vector3& scale);
    static Matrix4 FromTranslation(const Vector3& v);

    static Vector3 UnProject(const Vector3& vec, const Matrix4& view, const Matrix4& proj, const float viewport[]);
    static Vector3 Project(const Vector3& vec, const Matrix4& view, const Matrix4& proj, const float viewport[]);

    /// Column-major
    float m_[16];

    static const Matrix4 Identity;
};

}
