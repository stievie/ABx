#include "stdafx.h"
#include "Matrix4.h"

namespace Math {

const Matrix4 Matrix4::Identity;

Matrix4::Matrix4() noexcept
{
    // Identity Matrix
    *this = Matrix4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

Matrix4::Matrix4(
    float v00, float v01, float v02, float v03,
    float v10, float v11, float v12, float v13,
    float v20, float v21, float v22, float v23,
    float v30, float v31, float v32, float v33
) noexcept
{
    m_[Index00] = v00; m_[Index01] = v01; m_[Index02] = v02; m_[Index03] = v03;
    m_[Index10] = v10; m_[Index11] = v11; m_[Index12] = v12; m_[Index13] = v13;
    m_[Index20] = v20; m_[Index21] = v21; m_[Index22] = v22; m_[Index23] = v23;
    m_[Index30] = v30; m_[Index31] = v31; m_[Index32] = v32; m_[Index33] = v33;
}

Matrix4::Matrix4(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3) noexcept
{
    m_[Index00] = row0.x_; m_[Index01] = row0.y_; m_[Index02] = row0.z_; m_[Index03] = row0.w_;
    m_[Index10] = row1.x_; m_[Index11] = row1.y_; m_[Index12] = row1.z_; m_[Index13] = row1.w_;
    m_[Index20] = row2.x_; m_[Index21] = row2.y_; m_[Index22] = row2.z_; m_[Index23] = row2.w_;
    m_[Index30] = row3.x_; m_[Index31] = row3.y_; m_[Index32] = row3.z_; m_[Index33] = row3.w_;
}

#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
Matrix4::Matrix4(const XMath::XMMATRIX& matrix) noexcept
{
#if defined(HAVE_DIRECTX_MATH)
    // Row major
    m_[Index00] = matrix.r[0].m128_f32[0]; m_[Index01] = matrix.r[0].m128_f32[1]; m_[Index02] = matrix.r[0].m128_f32[2]; m_[Index03] = matrix.r[0].m128_f32[3];
    m_[Index10] = matrix.r[1].m128_f32[0]; m_[Index11] = matrix.r[1].m128_f32[1]; m_[Index12] = matrix.r[1].m128_f32[2]; m_[Index13] = matrix.r[1].m128_f32[3];
    m_[Index20] = matrix.r[2].m128_f32[0]; m_[Index21] = matrix.r[2].m128_f32[1]; m_[Index22] = matrix.r[2].m128_f32[2]; m_[Index23] = matrix.r[2].m128_f32[3];
    m_[Index30] = matrix.r[3].m128_f32[0]; m_[Index31] = matrix.r[3].m128_f32[1]; m_[Index32] = matrix.r[3].m128_f32[2]; m_[Index33] = matrix.r[3].m128_f32[3];
#elif defined(HAVE_X_MATH)
    // Col major
    m_[Index00] = matrix.c[0].m128_f32[0]; m_[Index01] = matrix.c[1].m128_f32[0]; m_[Index02] = matrix.c[2].m128_f32[0]; m_[Index03] = matrix.c[3].m128_f32[0];
    m_[Index10] = matrix.c[0].m128_f32[1]; m_[Index11] = matrix.c[1].m128_f32[1]; m_[Index12] = matrix.c[2].m128_f32[1]; m_[Index13] = matrix.c[3].m128_f32[1];
    m_[Index20] = matrix.c[0].m128_f32[2]; m_[Index21] = matrix.c[1].m128_f32[2]; m_[Index22] = matrix.c[2].m128_f32[2]; m_[Index23] = matrix.c[3].m128_f32[2];
    m_[Index30] = matrix.c[0].m128_f32[3]; m_[Index31] = matrix.c[1].m128_f32[3]; m_[Index32] = matrix.c[2].m128_f32[3]; m_[Index33] = matrix.c[3].m128_f32[3];
#endif
}
#endif

const Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
    XMath::XMMATRIX result = XMath::XMMatrixMultiply(
        XMath::XMMatrixSet(
            m_[0], m_[4], m_[8], m_[12],
            m_[1], m_[5], m_[9], m_[13],
            m_[2], m_[6], m_[10], m_[14],
            m_[3], m_[7], m_[11], m_[15]
        ),
        XMath::XMMatrixSet(
            rhs.m_[0], rhs.m_[4], rhs.m_[8], rhs.m_[12],
            rhs.m_[1], rhs.m_[5], rhs.m_[9], rhs.m_[13],
            rhs.m_[2], rhs.m_[6], rhs.m_[10], rhs.m_[14],
            rhs.m_[3], rhs.m_[7], rhs.m_[11], rhs.m_[15]
        )
    );
    return Matrix4(result);
}

const Vector3 Matrix4::operator*(const Vector3& rhs) const
{
    XMath::XMVECTOR v = XMath::XMVector3Transform(
        XMath::XMVectorSet(rhs.x_, rhs.y_, rhs.z_, 0.0f),
        XMath::XMMatrixSet(
            m_[0], m_[4], m_[8], m_[12],
            m_[1], m_[5], m_[9], m_[13],
            m_[2], m_[6], m_[10], m_[14],
            m_[3], m_[7], m_[11], m_[15]
        )
    );
    return Vector3(v);
}

const Vector4 Matrix4::operator*(const Vector4& rhs) const
{
    XMath::XMVECTOR v = XMath::XMVector3Transform(
        XMath::XMVectorSet(rhs.x_, rhs.y_, rhs.z_, rhs.w_),
        XMath::XMMatrixSet(
            m_[0], m_[4], m_[8], m_[12],
            m_[1], m_[5], m_[9], m_[13],
            m_[2], m_[6], m_[10], m_[14],
            m_[3], m_[7], m_[11], m_[15]
        )
    );
    return Vector4(v);
}

Matrix4& Matrix4::Translate(const Vector3& v)
{
    return *this = *this * Matrix4(
        1, 0, 0, v.x_,
        0, 1, 0, v.y_,
        0, 0, 1, v.z_,
        0, 0, 0, 1
    );
}

Matrix4& Matrix4::Scale(const Vector3& v)
{
    return *this = *this * Matrix4(
        v.x_, 0, 0, 0,
        0, v.y_, 0, 0,
        0, 0, v.z_, 0,
        0, 0, 0, 1
    );
}

Matrix4& Matrix4::RotateX(float ang)
{
    return *this = *this * Matrix4(
        1, 0, 0, 0,
        0, cos(ang), -sin(ang), 0,
        0, sin(ang), cos(ang), 0,
        0, 0, 0, 1
    );
}

Matrix4& Matrix4::RotateY(float ang)
{
    return *this = *this * Matrix4(
        cos(ang), 0, sin(ang), 0,
        0, 1, 0, 0,
        -sin(ang), 0, cos(ang), 0,
        0, 0, 0, 1
    );
}

Matrix4& Matrix4::RotateZ(float ang)
{
    return *this = *this * Matrix4(
        cos(ang), -sin(ang), 0, 0,
        sin(ang), cos(ang), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );
}

Matrix4& Matrix4::Rotate(const Vector3& axis, float ang)
{
    float s = sin(ang);
    float c = cos(ang);
    float t = 1 - c;
    Vector3 a = axis.Normal();

    return *this = *this * Matrix4(
        a.x_ * a.x_ * t + c, a.x_ * a.y_ * t - a.z_ * s, a.x_ * a.z_ * t + a.y_ * s, 0,
        a.y_ * a.x_ * t + a.z_ * s, a.y_ * a.y_ * t + c, a.y_ * a.z_ * t - a.x_ * s, 0,
        a.z_ * a.x_ * t - a.y_ * s, a.z_ * a.y_ * t + a.x_ * s, a.z_ * a.z_ * t + c, 0,
        0, 0, 0, 1
    );
}

Matrix4 Matrix4::Transpose() const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    return XMath::XMMatrixTranspose(*this);
#else
    Matrix4 res;

    res.m_[0] = m_[0];
    res.m_[1] = m_[4];
    res.m_[2] = m_[8];
    res.m_[3] = m_[12];

    res.m_[4] = m_[1];
    res.m_[5] = m_[5];
    res.m_[6] = m_[9];
    res.m_[7] = m_[13];

    res.m_[8] = m_[2];
    res.m_[9] = m_[6];
    res.m_[10] = m_[10];
    res.m_[11] = m_[14];

    res.m_[12] = m_[3];
    res.m_[13] = m_[7];
    res.m_[14] = m_[11];
    res.m_[15] = m_[15];

    return res;
#endif
}

float Matrix4::Determinant() const
{
    return m_[12] * m_[9] * m_[6] * m_[3] - m_[8] * m_[13] * m_[6] * m_[3] - m_[12] * m_[5] * m_[10] * m_[3] + m_[4] * m_[13] * m_[10] * m_[3] +
        m_[8] * m_[5] * m_[14] * m_[3] - m_[4] * m_[9] * m_[14] * m_[3] - m_[12] * m_[9] * m_[2] * m_[7] + m_[8] * m_[13] * m_[2] * m_[7] +
        m_[12] * m_[1] * m_[10] * m_[7] - m_[0] * m_[13] * m_[10] * m_[7] - m_[8] * m_[1] * m_[14] * m_[7] + m_[0] * m_[9] * m_[14] * m_[7] +
        m_[12] * m_[5] * m_[2] * m_[11] - m_[4] * m_[13] * m_[2] * m_[11] - m_[12] * m_[1] * m_[6] * m_[11] + m_[0] * m_[13] * m_[6] * m_[11] +
        m_[4] * m_[1] * m_[14] * m_[11] - m_[0] * m_[5] * m_[14] * m_[11] - m_[8] * m_[5] * m_[2] * m_[15] + m_[4] * m_[9] * m_[2] * m_[15] +
        m_[8] * m_[1] * m_[6] * m_[15] - m_[0] * m_[9] * m_[6] * m_[15] - m_[4] * m_[1] * m_[10] * m_[15] + m_[0] * m_[5] * m_[10] * m_[15];
}

Matrix4 Matrix4::Inverse() const
{
#if defined(HAVE_DIRECTX_MATH) || defined(HAVE_X_MATH)
    XMath::XMVECTOR det;
    return XMath::XMMatrixInverse(&det, *this);
#else
    float det = Determinant();

    Matrix4 res;

    float t0 = m_[0] * m_[5] - m_[1] * m_[4];
    float t1 = m_[0] * m_[6] - m_[2] * m_[4];
    float t2 = m_[0] * m_[7] - m_[3] * m_[4];
    float t3 = m_[1] * m_[6] - m_[2] * m_[5];
    float t4 = m_[1] * m_[7] - m_[3] * m_[5];
    float t5 = m_[2] * m_[7] - m_[3] * m_[6];
    float t6 = m_[8] * m_[13] - m_[9] * m_[12];
    float t7 = m_[8] * m_[14] - m_[10] * m_[12];
    float t8 = m_[8] * m_[15] - m_[11] * m_[12];
    float t9 = m_[9] * m_[14] - m_[10] * m_[13];
    float t10 = m_[9] * m_[15] - m_[11] * m_[13];
    float t11 = m_[10] * m_[15] - m_[11] * m_[14];

    res.m_[0] = (m_[5] * t11 - m_[6] * t10 + m_[7] * t9) / det;
    res.m_[1] = (-m_[1] * t11 + m_[2] * t10 - m_[3] * t9) / det;
    res.m_[2] = (m_[13] * t5 - m_[14] * t4 + m_[15] * t3) / det;
    res.m_[3] = (-m_[9] * t5 + m_[10] * t4 - m_[11] * t3) / det;
    res.m_[4] = (-m_[4] * t11 + m_[6] * t8 - m_[7] * t7) / det;
    res.m_[5] = (m_[0] * t11 - m_[2] * t8 + m_[3] * t7) / det;
    res.m_[6] = (-m_[12] * t5 + m_[14] * t2 - m_[15] * t1) / det;
    res.m_[7] = (m_[8] * t5 - m_[10] * t2 + m_[11] * t1) / det;
    res.m_[8] = (m_[4] * t10 - m_[5] * t8 + m_[7] * t6) / det;
    res.m_[9] = (-m_[0] * t10 + m_[1] * t8 - m_[3] * t6) / det;
    res.m_[10] = (m_[12] * t4 - m_[13] * t2 + m_[15] * t0) / det;
    res.m_[11] = (-m_[8] * t4 + m_[9] * t2 - m_[11] * t0) / det;
    res.m_[12] = (-m_[4] * t9 + m_[5] * t7 - m_[6] * t6) / det;
    res.m_[13] = (m_[0] * t9 - m_[1] * t7 + m_[2] * t6) / det;
    res.m_[14] = (-m_[12] * t3 + m_[13] * t1 - m_[14] * t0) / det;
    res.m_[15] = (m_[8] * t3 - m_[9] * t1 + m_[10] * t0) / det;

    return res;
#endif
}

Matrix4 Matrix4::FromFrustum(float left, float right, float bottom, float top, float _near, float _far)
{
    Matrix4 res;

    res.m_[0] = _near * 2.0f / (right - left);
    res.m_[5] = _near * 2.0f / (top - bottom);
    res.m_[8] = (right + left) / (right - left);
    res.m_[9] = (top + bottom) / (top - bottom);
    res.m_[10] = (-_far - _near) / (_far - _near);
    res.m_[11] = -1;
    res.m_[14] = -2.0f * _far * _near / (_far - _near);
    res.m_[15] = 0;

    return res;
}

Matrix4 Matrix4::FromPerspective(float fovy, float aspect, float _near, float _far)
{
    float top = _near * tan(fovy / 2.0f);
    float right = top * aspect;
    return FromFrustum(-right, right, -top, top, _near, _far);
}

Matrix4 Matrix4::FromOrtho(float left, float right, float bottom, float top, float _near, float _far)
{
    Matrix4 res;

    res.m_[0] = 2 / (right - left);
    res.m_[5] = 2 / (top - bottom);
    res.m_[10] = -2 / (_far - _near);
    res.m_[12] = -(left + right) / (right - left);
    res.m_[13] = -(top + bottom) / (top - bottom);
    res.m_[14] = -(_far + _near) / (_far - _near);

    return res;
}

Matrix4 Matrix4::FromOrtho(float width, float height, float _near, float _far)
{
    return Matrix4::FromOrtho(-width / 2, width / 2, -height / 2, height / 2, _near, _far);
}

Matrix4 Matrix4::FromQuaternion(const Quaternion& q)
{
    Quaternion _q = q.Normal();
    Vector4 axixAngle = _q.AxisAngle();
    return Matrix4::FromAxisAngle(Vector3(axixAngle.x_, axixAngle.y_, axixAngle.z_), axixAngle.w_);
}

Matrix4 Matrix4::FromAxisAngle(const Vector3& axis, float angle)
{
    Vector3 _axis = axis.Normal();
    float axisX = _axis.x_, axisY = _axis.y_, axisZ = _axis.z_;

    // Angles
    float _cos = cos(-angle);
    float _sin = sin(-angle);
    float t = 1.0f - _cos;

    // do the conversion math once
    float tXX = t * axisX * axisX,
        tXY = t * axisX * axisY,
        tXZ = t * axisX * axisZ,
        tYY = t * axisY * axisY,
        tYZ = t * axisY * axisZ,
        tZZ = t * axisZ * axisZ;

    float sinX = _sin * axisX,
        sinY = _sin * axisY,
        sinZ = _sin * axisZ;

    return Matrix4(
        Vector4(tXX + _cos, tXY - sinZ, tXZ + sinY, 0),
        Vector4(tXY + sinZ, tYY + _cos, tYZ - sinX, 0),
        Vector4(tXZ - sinY, tYZ + sinX, tZZ + _cos, 0),
        Vector4::UnitW
    );
}

Matrix4 Matrix4::FromScale(const Vector3& scale)
{
    Matrix4 m;
    m.Scale(scale);
    return m;
}

Matrix4 Matrix4::FromTranslation(const Vector3& v)
{
    Matrix4 m(Matrix4::Identity);
    m.m_[3] = v.x_;
    m.m_[7] = v.y_;
    m.m_[11] = v.z_;
    return m;
}

Quaternion Matrix4::Rotation(bool rowNormalize /* = true */) const
{
    Vector4 row0(m_[Index00], m_[Index01], m_[Index02], m_[Index03]);
    Vector4 row1(m_[Index10], m_[Index11], m_[Index12], m_[Index13]);
    Vector4 row2(m_[Index20], m_[Index21], m_[Index22], m_[Index23]);
    if (rowNormalize)
    {
        row0.Normalize();
        row1.Normalize();
        row2.Normalize();
    }
    Quaternion q;
    float trace = 0.25f * (row0.x_, row1.y_, row2.z_ + 1.0f);
    if (trace > 0)
    {
        float sq = sqrt(trace);
        q.w_ = sq;
        sq = 1.0f / (4.0f * sq);
        q.x_ = (row1.z_ - row2.y_) * sq;
        q.y_ = (row2.x_ - row0.z_) * sq;
        q.z_ = (row0.y_ - row1.x_) * sq;
    }
    else if (row0.x_ > row1.y_ && row0.x_ > row2.z_)
    {
        float sq = 2.0f * sqrt(1.0f + row0.x_ - row1.y_ - row2.z_);
        q.x_ = 0.25f * sq;
        sq = 1.0f / sq;
        q.w_ = (row2.y_ - row1.z_) * sq;
        q.y_ = (row1.x_ + row0.y_) * sq;
        q.z_ = (row2.x_ + row0.z_) * sq;
    }
    else if (row1.y_ > row2.z_)
    {
        float sq = 2.0f * sqrt(1.0f + row1.y_ - row0.x_ - row2.z_);
        q.y_ = 0.25f * sq;
        sq = 1.0f / sq;
        q.w_ = (row2.x_ - row0.z_) * sq;
        q.x_ = (row1.x_ + row0.y_) * sq;
        q.z_ = (row2.y_ + row1.z_) * sq;
    }
    else
    {
        float sq = 2.0f * sqrt(1.0f + row2.z_ - row0.x_ - row1.y_);
        q.z_ = 0.25f * sq;
        sq = 1.0f / sq;
        q.w_ = (row1.x_ - row0.y_) * sq;
        q.x_ = (row2.x_ + row0.z_) * sq;
        q.y_ = (row2.y_ + row1.z_) * sq;
    }
    return q.Normal();
}

Vector3 Matrix4::Translation() const
{
    return Vector3(
        m_[Index30],
        m_[Index31],
        m_[Index32]
    );
}

Vector3 Matrix4::Scaling() const
{
    return Vector3(
        sqrtf(m_[Index00] * m_[Index00] + m_[Index01] * m_[Index01] + m_[Index02] * m_[Index02]),
        sqrtf(m_[Index10] * m_[Index10] + m_[Index11] * m_[Index11] + m_[Index12] * m_[Index12]),
        sqrtf(m_[Index20] * m_[Index20] + m_[Index21] * m_[Index21] + m_[Index22] * m_[Index22])
    );
}

Matrix4 Matrix4::FromLookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
{
    Matrix4 res;

    Vector3 Z = (eye - center).Normal();

    Vector3 X = Vector3(
        up.y_ * Z.z_ - up.z_ * Z.y_,
        up.z_ * Z.x_ - up.x_ * Z.z_,
        up.x_ * Z.y_ - up.y_ * Z.x_
    ).Normal();

    Vector3 Y = Vector3(
        Z.y_ * X.z_ - Z.z_ * X.y_,
        Z.z_ * X.x_ - Z.x_ * X.z_,
        Z.x_ * X.y_ - Z.y_ * X.x_
    ).Normal();

    res.m_[0] = X.x_;
    res.m_[1] = Y.x_;
    res.m_[2] = Z.x_;
    res.m_[4] = X.y_;
    res.m_[5] = Y.y_;
    res.m_[6] = Z.y_;
    res.m_[8] = X.z_;
    res.m_[9] = Y.z_;
    res.m_[10] = Z.z_;
    res.m_[12] = -X.DotProduct(eye);
    res.m_[13] = -Y.DotProduct(eye);
    res.m_[14] = -Z.DotProduct(eye);

    return res;
}

Vector3 Matrix4::UnProject(const Vector3& vec, const Matrix4& view, const Matrix4& proj, const float viewport[])
{
    Matrix4 inv = (proj * view).Inverse();
    Vector3 v(
        (vec.x_ - viewport[0]) * 2.0f / viewport[2] - 1.0f,
        (vec.y_ - viewport[1]) * 2.0f / viewport[3] - 1.0f,
        2.0f * vec.z_ - 1.0f
    );

    Vector3 res = inv * v;
    float w = inv.m_[3] * v.x_ + inv.m_[7] * v.y_ + inv.m_[11] * v.z_ + inv.m_[15];

    return res / w;
}

Vector3 Matrix4::Project(const Vector3& vec, const Matrix4& view, const Matrix4& proj, const float viewport[])
{
    Matrix4 trans = proj * view;
    Vector3 v = trans * vec;

    float w = trans.m_[3] * vec.x_ + trans.m_[7] * vec.y_ + trans.m_[11] * vec.z_ + trans.m_[15];
    v = v / w;

    return Vector3(
        viewport[0] + viewport[2] * (v.x_ + 1.0f) / 2.0f,
        viewport[1] + viewport[3] * (v.y_ + 1.0f) / 2.0f,
        (v.z_ + 1.0f) / 2.0f
    );
}

}
