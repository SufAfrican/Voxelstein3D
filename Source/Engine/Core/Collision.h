#pragma once

/// Quickly tests if ray and bounding box intersect
inline bool TestRayBox(const Vector3D &vRayOrigin, const Vector3D &vRayDir, float fRayLength, const Vector3D &vBoxPos, const Vector3D &vBoxHalfSize)
{
  Vector3D vHalfRay = vRayDir * fRayLength * 0.5f;
  Vector3D vRayCenter = vRayOrigin + vHalfRay;
  Vector3D vAbsHalfRay(fabsf(vHalfRay.x), fabsf(vHalfRay.y), fabsf(vHalfRay.z));

  float fDx = vRayCenter.x - vBoxPos.x;
  if(fabsf(fDx) > vBoxHalfSize.x + vAbsHalfRay.x)
    return false;
  float fDy = vRayCenter.y - vBoxPos.y;
  if(fabsf(fDy) > vBoxHalfSize.y + vAbsHalfRay.y)
    return false;
  float fDz = vRayCenter.z - vBoxPos.z;
  if(fabsf(fDz) > vBoxHalfSize.z + vAbsHalfRay.z)
    return false;

  float f;
  f = vHalfRay.y * fDz - vHalfRay.z * fDy;
  if(fabsf(f) > vBoxHalfSize.y * vAbsHalfRay.z + vBoxHalfSize.z * vAbsHalfRay.y)
    return false;
  f = vHalfRay.z * fDx - vHalfRay.x * fDz;
  if(fabsf(f) > vBoxHalfSize.x * vAbsHalfRay.z + vBoxHalfSize.z * vAbsHalfRay.x)
    return false;
  f = vHalfRay.x * fDy - vHalfRay.y * fDx;
  if(fabsf(f) > vBoxHalfSize.x * vAbsHalfRay.y + vBoxHalfSize.y * vAbsHalfRay.x)
    return false;

  return true;
}

/// Sweep collision test box vs. box
inline bool SweepBoxBox(const Vector3D &vBox1Pos, const Vector3D &vBox1HalfSize,
                           const Vector3D &vBox2Pos, const Vector3D &vBox2HalfSize,
                           const Vector3D &vSweepDir,
                           float &fHitDistance, int &iHitAxis)
{
  Vector3D vBox1Start = vBox1Pos - vBox1HalfSize;
  Vector3D vBox1End = vBox1Pos + vBox1HalfSize;
  Vector3D vBox2Start = vBox2Pos - vBox2HalfSize;
  Vector3D vBox2End = vBox2Pos + vBox2HalfSize;

  float fMergedMinT = 0;
  float fMergedMaxT = FLT_MAX;

  iHitAxis = -1;

  // solve sweep range in all axis
  for(int iAxis = 0; iAxis < 3; iAxis++)
  {
    // get values for current axis
    const float &fBox1Start = vBox1Start[iAxis];
    const float &fBox1End = vBox1End[iAxis];
    const float &fBox2Start = vBox2Start[iAxis];
    const float &fBox2End = vBox2End[iAxis];
    const float &fSweepDir = vSweepDir[iAxis];

    // collision occurs when these two apply: (in all axes)
    // 1. fBox1End + t * fSweepDir >= fBox2Start
    // 2. fBox1Start + t * fSweepDir <= fBox2End

    // sweep test in this axis
    if(fSweepDir != 0)
    {
      // calculate t range
      float fMinT = (fBox2Start - fBox1End) / fSweepDir;
      float fMaxT = (fBox2End - fBox1Start) / fSweepDir;

      if(fMinT > fMaxT)
      {
        // swap min and max
        float fTemp = fMaxT;
        fMaxT = fMinT;
        fMinT = fTemp;
      }
      
      // ranges do not overlap
      if(fMinT > fMergedMaxT || fMaxT < fMergedMinT)
        return false;

      // merge ranges
      if(fMinT >= fMergedMinT)
      {
        fMergedMinT = fMinT;
        iHitAxis = iAxis;
      }
      if(fMaxT < fMergedMaxT)
      {
        fMergedMaxT = fMaxT;
      }
    }
    // normal overlap test
    else
    {
      // no collision can occur at all
      if(fBox1End < fBox2Start || fBox1Start > fBox2End)
        return false;
    }
  }

  // collides
  if(iHitAxis != -1)
  {
    fHitDistance = fMergedMinT;
    return true;
  }

  // no collision
  return false;
}

/// Ray vs. box intersection
inline bool IntersectRayBox(const Vector3D &vRayPos, const Vector3D &vRayDir,
                           const Vector3D &vBoxPos, const Vector3D &vBoxHalfSize,
                           float &fHitDistance, int &iHitAxis)
{
  Vector3D vBoxStart = vBoxPos - vBoxHalfSize;
  Vector3D vBoxEnd = vBoxPos + vBoxHalfSize;
  Vector3D vRelBoxPos = vBoxPos - vRayPos;

  float fMergedMinT = 0;
  float fMergedMaxT = FLT_MAX;

  iHitAxis = -1;

  // solve intersection in all axis
  for(int iAxis = 0; iAxis < 3; iAxis++)
  {
    // get values for current axis
    const float &fPos = vRayPos[iAxis];
    const float &fDir = vRayDir[iAxis];
    const float &fBoxStart = vBoxStart[iAxis];
    const float &fBoxEnd = vBoxEnd[iAxis];

    // collision occurs when these two apply: (in all axes)
    // 1. fPos + t * fDir >= fBoxStart
    // 2. fPos + t * fDir <= fBoxEnd

    // sweep test in this axis
    if(fDir != 0)
    {
      // calculate t range
      float fMinT = (fBoxStart - fPos) / fDir;
      float fMaxT = (fBoxEnd - fPos) / fDir;

      if(fMinT > fMaxT)
      {
        // swap min and max
        float fTemp = fMaxT;
        fMaxT = fMinT;
        fMinT = fTemp;
      }
      
      // ranges do not overlap
      if(fMinT > fMergedMaxT || fMaxT < fMergedMinT)
        return false;

      fMinT = Max(fMinT, 0.0f);

      // merge ranges
      if(fMinT >= fMergedMinT)
      {
        fMergedMinT = fMinT;
        iHitAxis = iAxis;
      }
      if(fMaxT < fMergedMaxT)
      {
        fMergedMaxT = fMaxT;
      }
    }
    // normal overlap test
    else
    {
      // no collision can occur at all
      if(fPos < fBoxStart || fPos > fBoxEnd)
        return false;
    }
  }

  // collides
  if(iHitAxis != -1)
  {
    fHitDistance = fMergedMinT;
    return true;
  }

  // no collision
  return false;
}