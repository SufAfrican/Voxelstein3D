#pragma once

/// Linearly interpolates two floats
inline float LerpFloat(float f0, float f1, float fFactor)
{
  return f0 + (f1 - f0)*fFactor;
}

/// Linearly interpolates two angles (radians)
inline float LerpRadians(float a0, float a1, float fFactor)
{
  // calculate delta
  float aDelta = WrapRadians(a1) - WrapRadians(a0);
  // adjust delta not to wrap around 360
  if (aDelta > Math::fPi)
  {
    aDelta -= 2 * Math::fPi;
  }
  else if (aDelta < - Math::fPi)
  {
    aDelta += 2 * Math::fPi;
  }
  // interpolate the delta
  return a0 + fFactor*aDelta;
}

/// Linearly interpolates two angles (degrees)
inline float LerpDegrees(float a0, float a1, float fFactor)
{
  // calculate delta
  float aDelta = WrapDegrees(a1) - WrapDegrees(a0);
  // adjust delta not to wrap around 360
  if (aDelta > 180)
  {
    aDelta -= 360;
  }
  else if (aDelta < -180)
  {
    aDelta += 360;
  }
  // interpolate the delta
  return a0 + fFactor*aDelta;
}

/// Linearly interpolates two 2D vectors
inline Vector2D LerpVector2D(const Vector2D &vOld, const Vector2D &vNew, float fFactor)
{
  Vector2D vRes;
  vRes.x = LerpFloat(vOld.x, vNew.x, fFactor);
  vRes.y = LerpFloat(vOld.y, vNew.y, fFactor);
  return vRes;
}

/// Linearly interpolates two 3D vectors
inline Vector3D LerpVector3D(const Vector3D &vOld, const Vector3D &vNew, float fFactor)
{
  Vector3D vRes;
  vRes.x = LerpFloat(vOld.x, vNew.x, fFactor);
  vRes.y = LerpFloat(vOld.y, vNew.y, fFactor);
  vRes.z = LerpFloat(vOld.z, vNew.z, fFactor);
  return vRes;
}

/// Spherical interpolation for two quaternions
inline Quaternion Slerp(Quaternion q1, const Quaternion &q2, float fFactor)
{
  float fABres = Dot(q1, q2);

  if(fABres < 0.0f) {
	  q1.w = - q1.w;
    q1.x = - q1.x;
    q1.y = - q1.y;
    q1.z = - q1.z;
	  fABres = Dot(q1, q2);
  }

  float theta = (float)acosf(fABres);
  if(fabs(theta) < FLT_MIN|| _isnan(theta)) return(q1);
  float fFactor1 = sinf(theta - fFactor*theta) / sinf(theta);
  float fFactor2 = sinf(fFactor * theta) / sinf(theta);
  Quaternion q3;
  q3.x = q1.x * fFactor1 + q2.x * fFactor2;
  q3.y = q1.y * fFactor1 + q2.y * fFactor2;
  q3.z = q1.z * fFactor1 + q2.z * fFactor2;
  q3.w = q1.w * fFactor1 + q2.w * fFactor2;
  return q3;
}
