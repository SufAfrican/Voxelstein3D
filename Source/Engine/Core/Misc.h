#pragma once

/// Converts radians to degrees
__forceinline float RadiansToDegrees(float fRadians) { return fRadians * 57.295779513082320876798154814105f; }
/// Converts degrees to radians
__forceinline float DegreesToRadians(float fDegrees) { return fDegrees * 0.017453292519943295769236907684886f; }

namespace Math
{
  /// Pi is a nice constant
  static const float fPi = 3.14159265358979323846264338328f;
}

/// Clamps variable in range [min..max]
template<class Type> inline Type Clamp(const Type &v, const Type &min, const Type &max)
{
  if(v > max) return max;
  if(v < min) return min;
  return v;
}

/// Fast round from float to integer
inline int Round(float x)
{
  int n;
  __asm fld x;
  __asm fistp n;
  return n;
}

/// Ensures angle is in range [0..2*Pi]
inline float WrapRadians(float a)
{
  return fmod( fmod(a,2 * Math::fPi) + 2 * Math::fPi, 2 * Math::fPi);
}

/// Ensures angle is in range [0..360]
inline float WrapDegrees(float a)
{
  return fmod( fmod(a,360) + 360, 360);
}

/// Returns the larger of the two given variables
template<class Type> inline Type Max(const Type &a, const Type &b)
{
  if(a > b) return a;
  return b;
}

/// Returns the smaller of the two given variables
template<class Type> inline Type Min(const Type &a, const Type &b)
{
  if(a < b) return a;
  return b;
}

/// Returns nearest power of two
inline float GetNearestPowerOfTwo(float a)
{
  const float fLog2 = 0.69314718055994530941723212145818f;
  return powf(2.0f, Round(logf(a) / fLog2));
}

/// Returns nearest power of x
inline float GetNearestPowerOf(float a, float x)
{
  return powf(x, Round(logf(a) / logf(x)));
}

/// Returns a random integer in given range
inline int GetRandomInt(int iMin, int iMax)
{
  return rand()%(1+iMax-iMin) + iMin;
}

/// Returns a random float in given range
inline float GetRandomFloat(float fMin, float fMax)
{
  return (rand() / (float)RAND_MAX) * (fMax - fMin) + fMin;
}

/// Returns the sign
template<class Type> inline Type Sign(Type a)
{
  if(a < 0)
    return -1;
  return 1;
}

/// Returns either the sign or 0
template<class Type> inline Type SignOrZero(Type a)
{
  if(a < 0)
    return -1;
  if(a > 0)
    return 1;
  return 0;
}
