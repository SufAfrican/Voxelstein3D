#pragma once

/// 2D vector class
typedef struct Vector2D
{
  union
  {
    struct { float x, y; };
    float v[2];
  };

  inline Vector2D() {}
  inline Vector2D(float fX, float fY) : x(fX), y(fY) {}

  inline bool operator == (const Vector2D &vOther) { return (x == vOther.x && y == vOther.y); }
  inline bool operator != (const Vector2D &vOther) { return (x != vOther.x || y != vOther.y); }
  inline Vector2D &operator /= (const float fScalar) { x /= fScalar; y /= fScalar; return (*this); }
  inline Vector2D &operator *= (const float fScalar) { x *= fScalar; y *= fScalar; return (*this); }
  inline Vector2D &operator += (const Vector2D &vOther) { x += vOther.x;  y += vOther.y; return (*this); }
  inline Vector2D &operator -= (const Vector2D &vOther) { x -= vOther.x;  y -= vOther.y; return (*this);  }

  inline friend Vector2D operator * (float k, const Vector2D &vOther) {  return Vector2D(vOther.x * k, vOther.y * k); }
  inline Vector2D operator * (float fScalar) const { return Vector2D(x * fScalar, y * fScalar); }
  inline Vector2D operator / (float fScalar) const { return Vector2D(x / fScalar, y / fScalar); }
  inline Vector2D operator + (const Vector2D &vOther) const { return Vector2D(x + vOther.x, y + vOther.y); }
  inline Vector2D operator - (const Vector2D &vOther) const { return Vector2D(x - vOther.x, y - vOther.y); }
  inline Vector2D operator - (void) const { return Vector2D(-x, -y); }

  inline float &operator[](int i) { return v[i]; }
  inline const float &operator[](int i) const { return v[i]; }

  /// Calculate length of vector
  inline float Length(void) const { return (float) sqrt(x * x + y * y); }

  /// Normalize vector (returns length)
  inline float Normalize(void)
  {
    float fLength = Length();
    if (fLength == 0.0f) return 0.0f;
    float fInvLength = 1.0f / fLength;
    (*this) *= fInvLength;
    return fLength;
  }
} Vector2D;

/// Normalize vector (returns length)
inline Vector2D Normalize(const Vector2D &vOther)
{
  float fLength = vOther.Length();
  if (fLength == 0.0f) return vOther;
  float fInvLength = 1.0f / fLength;
  return vOther * fInvLength;
}

/// Calculate cross product
inline float Cross(const Vector2D &vA, const Vector2D &vB)
{
  return (vA.x * vB.y) - (vA.y * vB.x);
}

/// Calculate dot product
inline float Dot(const Vector2D &vA, const Vector2D &vB)
{
  return (vA.x * vB.x) + (vA.y * vB.y);
}
