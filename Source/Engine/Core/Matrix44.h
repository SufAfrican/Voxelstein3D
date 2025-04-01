#pragma once

/// 4x4 matrix class
typedef struct Matrix44
{
  union
  {
    struct
    {
      float _11, _12, _13, _14;
      float _21, _22, _23, _24;
      float _31, _32, _33, _34;
      float _41, _42, _43, _44;
    };
    float m[4][4];
  };

  inline Matrix44() {}
  inline Matrix44( float f11, float f12, float f13, float f14,
                   float f21, float f22, float f23, float f24,
                   float f31, float f32, float f33, float f34,
                   float f41, float f42, float f43, float f44 ) :
    _11(f11), _12(f12), _13(f13), _14(f14),
    _21(f21), _22(f22), _23(f23), _24(f24),
    _31(f31), _32(f32), _33(f33), _34(f34),
    _41(f41), _42(f42), _43(f43), _44(f44) {}

  /// Copy 3x3 elements from other matrix
  inline void Set33(const Matrix44 &mOther)
  {
    for(int i = 0; i < 3; i++)
      for(int j = 0; j < 3; j++)
        m[i][j] = mOther.m[i][j];
  }

  /// Set matrix to identity
  inline void SetIdentity(void) { (*this) = Matrix44::mIdentity; }
  /// Fill matrix with zeroes
  inline void SetZero(void) { (*this) = Matrix44::mZero; }
  /// Set rotation from given euler angles (clears _11 to _33)
  inline void SetEuler(const Vector3D &vEuler);
  /// Set translation elements of matrix
  inline void SetTranslation(const Vector3D &vPos);
  /// Scale matrix with given vector
  inline void Scale(const Vector3D &v);
  /// Removes scaling
  inline void RemoveScale(void);

  /// Returns translation vector
  inline Vector3D GetTranslation(void) const;
  /// Returns scale vector
  inline Vector3D GetScale(void) const;
  /// Returns side vector
  inline Vector3D GetSideVector(void) const;
  /// Returns up vector
  inline Vector3D GetUpVector(void) const;
  /// Returns front vector
  inline Vector3D GetFrontVector(void) const;

  /// Identity matrix
  static const Matrix44 mIdentity;
  /// Zero matrix
  static const Matrix44 mZero;
} Matrix44;


/// Transforms vector with matrix
inline Vector3D Transform(const Matrix44 &M, const Vector3D &A)
{
  return Vector3D((A.x * M._11) + (A.y * M._21) + (A.z * M._31) + (1 * M._41),
                  (A.x * M._12) + (A.y * M._22) + (A.z * M._32) + (1 * M._42),
                  (A.x * M._13) + (A.y * M._23) + (A.z * M._33) + (1 * M._43));
}

/// Transforms vector with a rotation/scale matrix
inline Vector3D Transform3x3(const Matrix44 &M, const Vector3D &A)
{
  return Vector3D((A.x * M._11) + (A.y * M._21) + (A.z * M._31),
                  (A.x * M._12) + (A.y * M._22) + (A.z * M._32),
                  (A.x * M._13) + (A.y * M._23) + (A.z * M._33));
}

/// Multiplies two 4x3 matrices together
inline Matrix44 Multiply4x3(const Matrix44 &A, const Matrix44 &B)
{
  return Matrix44(
    (A._11 * B._11) + (A._12 * B._21) + (A._13 * B._31),
    (A._11 * B._12) + (A._12 * B._22) + (A._13 * B._32),
    (A._11 * B._13) + (A._12 * B._23) + (A._13 * B._33),
    0,

    (A._21 * B._11) + (A._22 * B._21) + (A._23 * B._31),
    (A._21 * B._12) + (A._22 * B._22) + (A._23 * B._32),
    (A._21 * B._13) + (A._22 * B._23) + (A._23 * B._33),
    0,

    (A._31 * B._11) + (A._32 * B._21) + (A._33 * B._31),
    (A._31 * B._12) + (A._32 * B._22) + (A._33 * B._32),
    (A._31 * B._13) + (A._32 * B._23) + (A._33 * B._33),
    0,

    (A._41 * B._11) + (A._42 * B._21) + (A._43 * B._31) + (A._44 * B._41),
    (A._41 * B._12) + (A._42 * B._22) + (A._43 * B._32) + (A._44 * B._42),
    (A._41 * B._13) + (A._42 * B._23) + (A._43 * B._33) + (A._44 * B._43),
    1);
}

/// Multiplies two matrices together
inline Matrix44 Multiply(const Matrix44 &A, const Matrix44 &B)
{
  return Matrix44(
    (A._11 * B._11) + (A._12 * B._21) + (A._13 * B._31) + (A._14 * B._41),
    (A._11 * B._12) + (A._12 * B._22) + (A._13 * B._32) + (A._14 * B._42),
    (A._11 * B._13) + (A._12 * B._23) + (A._13 * B._33) + (A._14 * B._43),
    (A._11 * B._14) + (A._12 * B._24) + (A._13 * B._34) + (A._14 * B._44),

    (A._21 * B._11) + (A._22 * B._21) + (A._23 * B._31) + (A._24 * B._41),
    (A._21 * B._12) + (A._22 * B._22) + (A._23 * B._32) + (A._24 * B._42),
    (A._21 * B._13) + (A._22 * B._23) + (A._23 * B._33) + (A._24 * B._43),
    (A._21 * B._14) + (A._22 * B._24) + (A._23 * B._34) + (A._24 * B._44),

    (A._31 * B._11) + (A._32 * B._21) + (A._33 * B._31) + (A._34 * B._41),
    (A._31 * B._12) + (A._32 * B._22) + (A._33 * B._32) + (A._34 * B._42),
    (A._31 * B._13) + (A._32 * B._23) + (A._33 * B._33) + (A._34 * B._43),
    (A._31 * B._14) + (A._32 * B._24) + (A._33 * B._34) + (A._34 * B._44),

    (A._41 * B._11) + (A._42 * B._21) + (A._43 * B._31) + (A._44 * B._41),
    (A._41 * B._12) + (A._42 * B._22) + (A._43 * B._32) + (A._44 * B._42),
    (A._41 * B._13) + (A._42 * B._23) + (A._43 * B._33) + (A._44 * B._43),
    (A._41 * B._14) + (A._42 * B._24) + (A._43 * B._34) + (A._44 * B._44));
}

/// Set translation elements of matrix
inline void Matrix44::SetTranslation(const Vector3D &vPos)
{
  _41 = vPos.x;
  _42 = vPos.y;
  _43 = vPos.z;
}

/// Returns translation vector
inline Vector3D Matrix44::GetTranslation(void) const
{
  return Vector3D(_41, _42, _43);
}

/// Returns side vector
inline Vector3D Matrix44::GetSideVector(void) const
{
  return Vector3D(_11, _12, _13);
}

/// Returns up vector
inline Vector3D Matrix44::GetUpVector(void) const
{
  return Vector3D(_21, _22, _23);
}

/// Returns front vector
inline Vector3D Matrix44::GetFrontVector(void) const
{
  return Vector3D(_31, _32, _33);
}

/// Returns scale vector
inline Vector3D Matrix44::GetScale(void) const
{
  return Vector3D(Vector3D(_11, _12, _13).Length(),
                  Vector3D(_21, _22, _23).Length(),
                  Vector3D(_31, _32, _33).Length());
}

/// Scale matrix with given vector
inline void Matrix44::Scale(const Vector3D &v)
{
  _11 *= v.x;
  _12 *= v.x;
  _13 *= v.x;
  _14 *= v.x;

  _21 *= v.y;
  _22 *= v.y;
  _23 *= v.y;
  _24 *= v.y;

  _31 *= v.z;
  _32 *= v.z;
  _33 *= v.z;
  _34 *= v.z;
}

/// Removes scaling
inline void Matrix44::RemoveScale(void)
{
  Vector3D v = GetScale();
  _11 /= v.x;
  _12 /= v.x;
  _13 /= v.x;
  _14 /= v.x;

  _21 /= v.y;
  _22 /= v.y;
  _23 /= v.y;
  _24 /= v.y;

  _31 /= v.z;
  _32 /= v.z;
  _33 /= v.z;
  _34 /= v.z;
}

/// Scale matrix with given vector
inline Matrix44 Scale(const Matrix44 &m, const Vector3D &v)
{
  return Matrix44(
    (v.x * m._11), (v.x * m._12), (v.x * m._13), (v.x * m._14),
    (v.y * m._21), (v.y * m._22), (v.y * m._23), (v.y * m._24),
    (v.z * m._31), (v.z * m._32), (v.z * m._33), (v.z * m._34),
      (1 * m._41),   (1 * m._42),   (1 * m._43),   (1 * m._44));
}

/// Set rotation from given euler angles (clears _11 to _33)
inline void Matrix44::SetEuler(const Vector3D &vEuler)
{
  // calculate from euler angles
  float ch = cosf(vEuler.x);
  float sh = sinf(vEuler.x);
  float cp = cosf(vEuler.y);
  float sp = sinf(vEuler.y);
  float cb = cosf(vEuler.z);
  float sb = sinf(vEuler.z);

  _11 = (cb * ch) + (sp * sh * sb);
  _12 = cp * sb;
  _13 = (-cb * sh) + (sp * ch * sb);
  _21 = (-sb * ch) + (sp * sh * cb);
  _22 = cp * cb;
  _23 = (sb * sh) + (sp * ch * cb);
  _31 = sh * cp;
  _32 = -sp;
  _33 = cp * ch;
}

/// Transpose matrix
inline Matrix44 Transpose(const Matrix44 &m)
{
  return Matrix44(m._11, m._21, m._31, m._41,
                  m._12, m._22, m._32, m._42,
                  m._13, m._23, m._33, m._43,
                  m._14, m._24, m._34, m._44);
}

/// Calculate inverse for rotation & translation only matrix
inline Matrix44 InverseFast(const Matrix44 &src)
{
  Matrix44 dest;
  dest.m[0][0] = src.m[0][0];
  dest.m[1][0] = src.m[0][1];
  dest.m[2][0] = src.m[0][2];
  dest.m[3][0] = -(src.m[3][0]*src.m[0][0] + src.m[3][1]*src.m[0][1] + src.m[3][2]*src.m[0][2]);

  dest.m[0][1] = src.m[1][0];
  dest.m[1][1] = src.m[1][1];
  dest.m[2][1] = src.m[1][2];
  dest.m[3][1] = -(src.m[3][0]*src.m[1][0] + src.m[3][1]*src.m[1][1] + src.m[3][2]*src.m[1][2]);

  dest.m[0][2] = src.m[2][0];
  dest.m[1][2] = src.m[2][1];
  dest.m[2][2] = src.m[2][2];
  dest.m[3][2] = -(src.m[3][0]*src.m[2][0] + src.m[3][1]*src.m[2][1] + src.m[3][2]*src.m[2][2]);

  dest.m[0][3] = 0.0f;
  dest.m[1][3] = 0.0f;
  dest.m[2][3] = 0.0f;
  dest.m[3][3] = 1.0f;
  return dest;
}

/// Calculate inverse for generic matrix
inline Matrix44 Inverse(const Matrix44 &m)
{
  float a0 = (m.m[0][0] * m.m[1][1]) - (m.m[0][1] * m.m[1][0]);
  float a1 = (m.m[0][0] * m.m[1][2]) - (m.m[0][2] * m.m[1][0]);
  float a2 = (m.m[0][0] * m.m[1][3]) - (m.m[0][3] * m.m[1][0]);
  float a3 = (m.m[0][1] * m.m[1][2]) - (m.m[0][2] * m.m[1][1]);
  float a4 = (m.m[0][1] * m.m[1][3]) - (m.m[0][3] * m.m[1][1]);
  float a5 = (m.m[0][2] * m.m[1][3]) - (m.m[0][3] * m.m[1][2]);
  float b0 = (m.m[2][0] * m.m[3][1]) - (m.m[2][1] * m.m[3][0]);
  float b1 = (m.m[2][0] * m.m[3][2]) - (m.m[2][2] * m.m[3][0]);
  float b2 = (m.m[2][0] * m.m[3][3]) - (m.m[2][3] * m.m[3][0]);
  float b3 = (m.m[2][1] * m.m[3][2]) - (m.m[2][2] * m.m[3][1]);
  float b4 = (m.m[2][1] * m.m[3][3]) - (m.m[2][3] * m.m[3][1]);
  float b5 = (m.m[2][2] * m.m[3][3]) - (m.m[2][3] * m.m[3][2]);

  float fDet = (a0 * b5) - (a1 * b4) + (a2 * b3) + (a3 * b2) - (a4 * b1) + (a5 * b0);

  // not possible to invert
  if (fabs(fDet) < 1e-06f)
    return m;

  float fInvDet = 1.0f / fDet;

  Matrix44 mInverse;
  mInverse.m[0][0] = + (m.m[1][1] * b5) - (m.m[1][2] * b4) + (m.m[1][3] * b3);
  mInverse.m[1][0] = - (m.m[1][0] * b5) + (m.m[1][2] * b2) - (m.m[1][3] * b1);
  mInverse.m[2][0] = + (m.m[1][0] * b4) - (m.m[1][1] * b2) + (m.m[1][3] * b0);
  mInverse.m[3][0] = - (m.m[1][0] * b3) + (m.m[1][1] * b1) - (m.m[1][2] * b0);
  mInverse.m[0][1] = - (m.m[0][1] * b5) + (m.m[0][2] * b4) - (m.m[0][3] * b3);
  mInverse.m[1][1] = + (m.m[0][0] * b5) - (m.m[0][2] * b2) + (m.m[0][3] * b1);
  mInverse.m[2][1] = - (m.m[0][0] * b4) + (m.m[0][1] * b2) - (m.m[0][3] * b0);
  mInverse.m[3][1] = + (m.m[0][0] * b3) - (m.m[0][1] * b1) + (m.m[0][2] * b0);
  mInverse.m[0][2] = + (m.m[3][1] * a5) - (m.m[3][2] * a4) + (m.m[3][3] * a3);
  mInverse.m[1][2] = - (m.m[3][0] * a5) + (m.m[3][2] * a2) - (m.m[3][3] * a1);
  mInverse.m[2][2] = + (m.m[3][0] * a4) - (m.m[3][1] * a2) + (m.m[3][3] * a0);
  mInverse.m[3][2] = - (m.m[3][0] * a3) + (m.m[3][1] * a1) - (m.m[3][2] * a0);
  mInverse.m[0][3] = - (m.m[2][1] * a5) + (m.m[2][2] * a4) - (m.m[2][3] * a3);
  mInverse.m[1][3] = + (m.m[2][0] * a5) - (m.m[2][2] * a2) + (m.m[2][3] * a1);
  mInverse.m[2][3] = - (m.m[2][0] * a4) + (m.m[2][1] * a2) - (m.m[2][3] * a0);
  mInverse.m[3][3] = + (m.m[2][0] * a3) - (m.m[2][1] * a1) + (m.m[2][2] * a0);

  mInverse.m[0][0] *= fInvDet;
  mInverse.m[1][0] *= fInvDet;
  mInverse.m[2][0] *= fInvDet;
  mInverse.m[3][0] *= fInvDet;

  mInverse.m[0][1] *= fInvDet;
  mInverse.m[1][1] *= fInvDet;
  mInverse.m[2][1] *= fInvDet;
  mInverse.m[3][1] *= fInvDet;

  mInverse.m[0][2] *= fInvDet;
  mInverse.m[1][2] *= fInvDet;
  mInverse.m[2][2] *= fInvDet;
  mInverse.m[3][2] *= fInvDet;

  mInverse.m[0][3] *= fInvDet;
  mInverse.m[1][3] *= fInvDet;
  mInverse.m[2][3] *= fInvDet;
  mInverse.m[3][3] *= fInvDet;

  return mInverse;
}

/// Make a rotation matrix from given euler angles
inline Matrix44 MatrixFromEuler(const Vector3D &vEuler)
{
  Matrix44 m = Matrix44::mIdentity;
  m.SetEuler(vEuler);
  return m;
}

/// Constructs a direction vector from rotation matrix
inline Vector3D GetDirectionVector(const Matrix44 &mRot)
{
  return Vector3D(mRot._31, mRot._32, mRot._33);
}

/// Constructs euler angles from rotation matrix
inline Vector3D GetEuler(const Matrix44 &mRot)
{
  // singularity at north pole
  if(mRot._32 > 0.998f)
    return Vector3D(-atan2f(mRot._13, mRot._11), -1.5707963267948966192313216916398f, 0.0f);

  // singularity at south pole
  if(mRot._32 < -0.998f)
	  return Vector3D(-atan2f(mRot._13, mRot._11), 1.5707963267948966192313216916398f, 0.0f);

  return Vector3D(atan2f(mRot._31, mRot._33), asinf(-mRot._32), atan2f(mRot._12, mRot._22));
}

/// Constructs euler angles from direction vector
inline Vector3D GetEuler(const Vector3D &vDir)
{
  return Vector3D(atan2f(vDir.x, vDir.z), asinf(-vDir.y), 0.0f);
}

/// Make a translation matrix from given translation
inline Matrix44 TranslationMatrix(const Vector3D &vTranslation)
{
  Matrix44 m = Matrix44::mIdentity;
  m.SetTranslation(vTranslation);
  return m;
}
