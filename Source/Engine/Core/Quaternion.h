#pragma once

/// 4D quaternion class
typedef struct Quaternion
{
  union
  {
    struct { float x, y, z, w; };
    struct { Vector3D xyz; float w; };
    float v[4];
  };

  Quaternion() {}
  Quaternion(float fX, float fY, float fZ, float fW) : x(fX), y(fY), z(fZ), w(fW) {}

  /// Quaternion multiplication
  inline Quaternion operator * (const Quaternion &q) const
  {
    return Quaternion((w * q.x) + (x * q.w) + (y * q.z) - (z * q.y),
                      (w * q.y) - (x * q.z) + (y * q.w) + (z * q.x),
                      (w * q.z) + (x * q.y) - (y * q.x) + (z * q.w),
                      (w * q.w) - (x * q.x) - (y * q.y) - (z * q.z));
  }

  inline Quaternion &operator *= (const Quaternion &q) { (*this) = (*this) * q; return (*this); }

  inline bool operator == (const Quaternion &q) { return (x == q.x && y == q.y && z == q.z && w == q.w); }
  inline bool operator != (const Quaternion &q) { return (x != q.x || y != q.y || z != q.z || w != q.w); }

  /// Normalize vector (returns length)
  inline float Normalize(void)
  {
    float fLength = (float) sqrt(x * x + y * y + z * z + w * w);
    if (fLength == 0.0f) return 0.0f;
    float fInvLength = 1.0f / fLength;
    x *= fInvLength;
    y *= fInvLength;
    z *= fInvLength;
    w *= fInvLength;
    return fLength;
  }

  /// Identity quaternion
  static const Quaternion qIdentity;
} Quaternion;

/// Create quaternion from a counter-clockwise rotation around an axis
inline Quaternion QuaternionFromEuler(const Vector3D &vEuler)
{
  float fSH = sinf(-vEuler.x * 0.5f);
  float fCH = cosf(-vEuler.x * 0.5f);
  float fSP = sinf(-vEuler.y * 0.5f);
  float fCP = cosf(-vEuler.y * 0.5f);
  float fSB = sinf(-vEuler.z * 0.5f);
  float fCB = cosf(-vEuler.z * 0.5f);
  Quaternion qH;
  qH.x = 0;
  qH.y = fSH;
  qH.z = 0;
  qH.w = fCH;

  Quaternion qP;
  qP.x = fSP;
  qP.y = 0;
  qP.z = 0;
  qP.w = fCP;

  Quaternion qB;
  qB.x = 0;
  qB.y = 0;
  qB.z = fSB;
  qB.w = fCB;

  // todo: optimize
  return qB * qP * qH;
}

/// Create quaternion from a counter-clockwise rotation around an axis
inline Quaternion QuaternionFromAxisAngle(const Vector3D &vAxis, float fAngleCCW)
{
  float fS = sinf(fAngleCCW * 0.5f);
  float fC = cosf(fAngleCCW * 0.5f);
  Quaternion q;
  q.x = fS * vAxis.x;
  q.y = fS * vAxis.y;
  q.z = fS * vAxis.z;
  q.w = fC;
  return q;
}

/// Convert a rotation matrix to a quaternion
inline Quaternion QuaternionFromMatrix(const Matrix44 &M)
{
  Quaternion q;
  q.w = sqrtf( Max( 0.0f, 1 + M._11 + M._22 + M._33 ) ) * 0.5f; 
  q.x = sqrtf( Max( 0.0f, 1 + M._11 - M._22 - M._33 ) ) * 0.5f; 
  q.y = sqrtf( Max( 0.0f, 1 - M._11 + M._22 - M._33 ) ) * 0.5f; 
  q.z = sqrtf( Max( 0.0f, 1 - M._11 - M._22 + M._33 ) ) * 0.5f; 
  q.x = _copysign( q.x, M._32 - M._23 );
  q.y = _copysign( q.y, M._13 - M._31 );
  q.z = _copysign( q.z, M._21 - M._12 );
  return q;
}

/// Convert a quaternion to a rotation matrix
inline Matrix44 MatrixFromQuaternion(const Quaternion &q)
{
  float xx = q.x * q.x;
  float xy = q.x * q.y;
  float xz = q.x * q.z;
  float xw = q.x * q.w;

  float yy = q.y * q.y;
  float yz = q.y * q.z;
  float yw = q.y * q.w;

  float zz = q.z * q.z;
  float zw = q.z * q.w;

  return Matrix44(  1 - 2 * ( yy + zz ),      2 * ( xy - zw ),      2 * ( xz + yw ),    0,
                        2 * ( xy + zw ),  1 - 2 * ( xx + zz ),      2 * ( yz - xw ),    0,
                        2 * ( xz - yw ),      2 * ( yz + xw ),  1 - 2 * ( xx + yy ),    0,
                                      0,                    0,                    0,    1 );
}

/// Constructs a direction vector from quaternion
inline Vector3D GetDirectionVector(const Quaternion &q)
{
  float xz = q.x * q.z;
  float yw = q.y * q.w;
  float yz = q.y * q.z;
  float xw = q.x * q.w;
  float xx = q.x * q.x;
  float yy = q.y * q.y;
  return Vector3D(2 * ( xz - yw ), 2 * ( yz + xw ),  1 - 2 * ( xx + yy ));
}

/// Dot product
inline float Dot(const Quaternion &A, const Quaternion &B)
{
  return A.w*B.w + A.x*B.x + A.y*B.y + A.z*B.z;
}

/// Quaternion inverse (conjugate)
inline Quaternion Inverse(const Quaternion &q)
{
  return Quaternion(-q.x, -q.y, -q.z, q.w);
}