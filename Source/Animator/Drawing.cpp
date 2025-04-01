#include "StdAfx.h"

#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"

#include "Engine/VoxlapConversions.h"

void DrawLine(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bDepthTest)
{
  point3d start = ConvertToVoxlap<point3d>(vStart);
  point3d end = ConvertToVoxlap<point3d>(vEnd);
  int iAlpha = 0;
  if(!bDepthTest)
    iAlpha = 0xFF;

  drawline3d(start.x, start.y, start.z,
    end.x, end.y, end.z,
    (iAlpha<<24) | ((iR&0xFF)<<16) | ((iG&0xFF)<<8) | (iB&0xFF));
}

void DrawArrow(const Vector3D &vStart, const Vector3D &vEnd, float fSize, const Vector3D &vFaceDir, int iR, int iG, int iB, bool bDepthTest)
{
  DrawLine(vStart, vEnd, iR, iG, iB,bDepthTest);

  Vector3D vDir = Normalize(vEnd - vStart);
  Vector3D vCross = Cross(vDir, vFaceDir);
  DrawLine(vEnd - vDir * fSize + vCross * fSize, vEnd, iR, iG, iB,bDepthTest);
  DrawLine(vEnd - vDir * fSize - vCross * fSize, vEnd, iR, iG, iB,bDepthTest);
}

void DrawCircle(const Vector3D &vCenter, const Vector3D &vDirX, const Vector3D &vDirY, float fRadius, float fSlices, int iR, int iG, int iB, bool bDepthTest)
{
  float fSliceStep = 2.0f * Math::fPi / fSlices;
  for(float fSlice = fSliceStep; fSlice < 2.0f * Math::fPi + fSliceStep*0.5f; fSlice += fSliceStep)
  {
    Vector3D vDir = GetDirectionVector(Vector3D(fSlice,0,0));
    Vector3D vDirLast = GetDirectionVector(Vector3D(fSlice - fSliceStep,0,0));

    DrawLine(vCenter + (vDirX * vDirLast.x + vDirY * vDirLast.z) * fRadius,
             vCenter + (vDirX * vDir.x     + vDirY * vDir.z) * fRadius,
             iR, iG, iB,bDepthTest);
  }
}

void DrawBoxLines(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bDepthTest)
{
  Vector3D vSize = vEnd - vStart;
  DrawLine(vStart, vStart + Vector3D(vSize.x,0,0), iR,iG,iB,bDepthTest);
  DrawLine(vStart, vStart + Vector3D(0,vSize.y,0), iR,iG,iB,bDepthTest);
  DrawLine(vStart, vStart + Vector3D(0,0,vSize.z), iR,iG,iB,bDepthTest);

  DrawLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(vSize.x,0,0),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(0,vSize.y,0),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bDepthTest);

  DrawLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(0,0,vSize.z),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(0,vSize.y,0),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bDepthTest);

  DrawLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(0,0,vSize.z),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(vSize.x,0,0),             iR,iG,iB,bDepthTest);
  DrawLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bDepthTest);
}

Vector3D GetWorldFromScreen(int iX, int iY, const Matrix44 &mCamera)
{
  int iSizeX, iSizeY;
  getscreensize(iSizeX, iSizeY);

  float fScreenWorldX = 1.0f;
  float fScreenWorldY = 1.0f * (iSizeY/(float)iSizeX);

  float fX = ((iX/(float)iSizeX) - 0.5f) * 2.0f;
  float fY = ((1.0f - iY/(float)iSizeY) - 0.5f) * 2.0f;
  float fZ = 1.0f;

  Vector3D vPos = mCamera.GetTranslation();
  // x position
  vPos += mCamera.GetSideVector() * fX * fScreenWorldX;
  // y position
  vPos += mCamera.GetUpVector() * fY * fScreenWorldY;
  // z position
  vPos += mCamera.GetFrontVector() * fZ;
  return vPos;
}
