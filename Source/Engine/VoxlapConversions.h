#pragma once

///////////////////////////////
// Game -> Voxlap conversions

/// Constructs a voxlap point from a Vector3D (no coordinate conversion)
template<typename T> inline T MakePoint(const Vector3D &vPoint)
{
  T pnt = {vPoint.x, vPoint.y, vPoint.z};
  return pnt;
}

/// Converts Vector3D to voxlap coordinates
template<typename T> inline T ConvertToVoxlap(const Vector3D &vPoint)
{
  T pnt = {-vPoint.x, vPoint.z, -vPoint.y};
  return pnt;
}

/// Converts Vector3D to voxlap coordinates, special case
template<> inline Vector3D ConvertToVoxlap<Vector3D>(const Vector3D &vPoint)
{
  return Vector3D(-vPoint.x, vPoint.z, -vPoint.y);
}

/// Converts a matrix to voxlap
inline Matrix44 ConvertToVoxlap(const Matrix44 &m)
{
  return Multiply(m, Matrix44(-1,0,0,0,  0,0,-1,0,  0,1,0,0, 0,0,0,1));
}

///////////////////////////////
// Voxlap -> Game conversions

/// Constructs a Vector3D from a voxlap point (no coordinate conversion)
template<typename T> inline Vector3D MakeVector(const T &pnt)
{
  return Vector3D(pnt.x, pnt.y, pnt.z);
}

/// Converts voxlap coordinates back to Vector3D
template<typename T> inline Vector3D ConvertFromVoxlap(const T &pnt)
{
  return Vector3D(-pnt.x, -pnt.z, pnt.y);
}

/// Converts voxlap coordinates back to Matrix44
template<typename T> Matrix44 ConvertFromVoxlap(const T &ipos, const T &istr, const T &ihei, const T &ifor) 
{
  Vector3D vPos = ConvertFromVoxlap<T>(ipos);
  Vector3D vSide = ConvertFromVoxlap<T>(istr);
  Vector3D vUp = -ConvertFromVoxlap<T>(ihei); // this is down vector
  Vector3D vFront = ConvertFromVoxlap<T>(ifor);
  return Matrix44(vSide.x, vSide.y, vSide.z, 0,
                  vUp.x, vUp.y, vUp.z, 0,
                  vFront.x, vFront.y, vFront.z, 0,
                  vPos.x, vPos.y, vPos.z, 1);
}
