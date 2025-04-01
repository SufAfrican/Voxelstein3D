#pragma once

/// 3D vector class
typedef struct Vector3D
{
  union
  {
    struct { float x, y, z; };
    struct { Vector2D xy; float z; };
    float v[3];
  };

  inline Vector3D() {}
  inline Vector3D(float fX, float fY, float fZ) : x(fX), y(fY), z(fZ) {}

  inline bool operator == (const Vector3D &vOther) { return (x == vOther.x && y == vOther.y && z == vOther.z); }
  inline bool operator != (const Vector3D &vOther) { return (x != vOther.x || y != vOther.y || z != vOther.z); }
  inline Vector3D &operator /= (const float fScalar) { x /= fScalar; y /= fScalar; z /= fScalar; return (*this); }
  inline Vector3D &operator *= (const float fScalar) { x *= fScalar; y *= fScalar; z *= fScalar; return (*this); }
  inline Vector3D &operator += (const Vector3D &vOther) { x += vOther.x;  y += vOther.y; z += vOther.z; return (*this); }
  inline Vector3D &operator -= (const Vector3D &vOther) { x -= vOther.x;  y -= vOther.y; z -= vOther.z; return (*this);  }

  inline friend Vector3D operator * (float k, const Vector3D &vOther) {  return Vector3D(vOther.x * k, vOther.y * k, vOther.z * k); }
  inline Vector3D operator * (float fScalar) const { return Vector3D(x * fScalar, y * fScalar, z * fScalar); }
  inline Vector3D operator / (float fScalar) const { return Vector3D(x / fScalar, y / fScalar, z / fScalar); }
  inline Vector3D operator + (const Vector3D &vOther) const { return Vector3D(x + vOther.x, y + vOther.y, z + vOther.z); }
  inline Vector3D operator - (const Vector3D &vOther) const { return Vector3D(x - vOther.x, y - vOther.y, z - vOther.z); }
  inline Vector3D operator - (void) const { return Vector3D(-x, -y, -z); }

  inline float &operator[](int i) { return v[i]; }
  inline const float &operator[](int i) const { return v[i]; }

  /// Returns 2D vector with x and z
  inline Vector2D GetXZ() const { return Vector2D(x,z); }

  /// Calculate length of vector
  inline float Length(void) const { return (float) sqrt(x * x + y * y + z * z); }
  inline float SquaredLength(void) const { return x * x + y * y + z * z; }

  /// Normalize vector (returns length)
  inline float Normalize(void)
  {
    float fLength = Length();
    if (fLength == 0.0f) return 0.0f;
    float fInvLength = 1.0f / fLength;
    (*this) *= fInvLength;
    return fLength;
  }
} Vector3D;

/// Normalize vector (returns length)
inline Vector3D Normalize(const Vector3D &vOther)
{
  float fLength = vOther.Length();
  if (fLength == 0.0f) return vOther;
  float fInvLength = 1.0f / fLength;
  return vOther * fInvLength;
}

/// Calculate cross product
inline Vector3D Cross(const Vector3D &vA, const Vector3D &vB)
{
  return Vector3D( (vA.y * vB.z) - (vA.z * vB.y),
                   (vA.z * vB.x) - (vA.x * vB.z),
                   (vA.x * vB.y) - (vA.y * vB.x) );
}

/// Calculate dot product
inline float Dot(const Vector3D &vA, const Vector3D &vB)
{
  return (vA.x * vB.x) + (vA.y * vB.y) + (vA.z * vB.z);
}

/// Constructs a direction vector from euler angles
inline Vector3D GetDirectionVector(const Vector3D &vEuler)
{
  float ch = cosf(vEuler.x);
  float sh = sinf(vEuler.x);
  float cp = cosf(vEuler.y);
  float sp = sinf(vEuler.y);
  return Vector3D(sh * cp, -sp, cp * ch);
}

/// Converts radians to degrees
__forceinline Vector3D RadiansToDegrees(const Vector3D &vRadians) { return Vector3D(RadiansToDegrees(vRadians.x),RadiansToDegrees(vRadians.y),RadiansToDegrees(vRadians.z)); }
/// Converts degrees to radians
__forceinline Vector3D DegreesToRadians(const Vector3D &vDegrees) { return Vector3D(DegreesToRadians(vDegrees.x),DegreesToRadians(vDegrees.y),DegreesToRadians(vDegrees.z)); }
/// Ensures angles are in range [0..2*Pi]
inline Vector3D WrapRadians(const Vector3D &vRadians) { return Vector3D(WrapRadians(vRadians.x), WrapRadians(vRadians.y), WrapRadians(vRadians.z)); }
/// Ensures angles are in range [0..360]
inline Vector3D WrapDegrees(const Vector3D &vDegrees) { return Vector3D(WrapDegrees(vDegrees.x), WrapDegrees(vDegrees.y), WrapDegrees(vDegrees.z)); }


