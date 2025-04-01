#include "StdAfx.h"

#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

extern void RegisterSprite(vx5sprite *sprite, bool bRenderInFront);
extern void UnregisterSprite(vx5sprite *sprite, bool bRenderInFront);

Sprite::Sprite(const std::string &strFile, bool bRenderInFront)
{
  m_bRenderInFront = bRenderInFront;
  m_strFilename = strFile;
  m_bOwnedData = true;
  m_bUniqueData = false;
  m_pData = new vx5sprite;

  // load sprite
  getspr(m_pData, strFile.c_str());
  // generate mips for KV6
	if (!(m_pData->flags&2))
	{
		kv6data *tempkv6 = m_pData->voxnum;
		while (tempkv6 = genmipkv6(tempkv6));
	}

  // load default transform
  m_mTransform = ConvertFromVoxlap<point3d>(m_pData->p, m_pData->s, m_pData->h, m_pData->f);

  // add for rendering
  RegisterSprite(m_pData, m_bRenderInFront);
  m_bVisible = true;
}

Sprite::Sprite(vx5sprite *pData, bool bRenderInFront)
{
  m_bRenderInFront = bRenderInFront;
  m_pData = pData;
  m_bOwnedData = false;
  m_bUniqueData = false;

  // load default transform
  m_mTransform = ConvertFromVoxlap<point3d>(m_pData->p, m_pData->s, m_pData->h, m_pData->f);

  // add for rendering
  RegisterSprite(m_pData, m_bRenderInFront);
  m_bVisible = true;
}

Sprite::~Sprite()
{
  // remove from rendering
  if(m_bVisible)
    UnregisterSprite(m_pData, m_bRenderInFront);
  if(m_bUniqueData)
    free(m_pData->voxnum);
  if(m_bOwnedData)
    delete m_pData;
}

void Sprite::SetTransform(const Matrix44 &mTrans)
{
  m_mTransform = mTrans;
  Matrix44 mConverted = ConvertToVoxlap(m_mTransform);
  m_pData->p.x = mConverted.GetTranslation().x;
  m_pData->p.y = mConverted.GetTranslation().y;
  m_pData->p.z = mConverted.GetTranslation().z;

  m_pData->s.x = mConverted.GetSideVector().x;
  m_pData->s.y = mConverted.GetSideVector().y;
  m_pData->s.z = mConverted.GetSideVector().z;

  // this is down vector
  m_pData->h.x = -mConverted.GetUpVector().x;
  m_pData->h.y = -mConverted.GetUpVector().y;
  m_pData->h.z = -mConverted.GetUpVector().z;

  m_pData->f.x = mConverted.GetFrontVector().x;
  m_pData->f.y = mConverted.GetFrontVector().y;
  m_pData->f.z = mConverted.GetFrontVector().z;
}

void Sprite::SetVisible(bool bVisible)
{
  if(m_bVisible && !bVisible)
    UnregisterSprite(m_pData, m_bRenderInFront);
  else if(!m_bVisible && bVisible)
    RegisterSprite(m_pData, m_bRenderInFront);
  m_bVisible = bVisible;
}

void Sprite::SetRenderInFront(bool bRenderInFront)
{
  if(m_bVisible)
    UnregisterSprite(m_pData, m_bRenderInFront);

  m_bRenderInFront = bRenderInFront;

  if(m_bVisible)
    RegisterSprite(m_pData, m_bRenderInFront);
}

void Sprite::CalculateBoundingBox(Vector3D &vMin, Vector3D &vMax, bool bTransformed, bool bAccurate)
{
  int iNumX = m_pData->voxnum->xsiz;
  int iNumY = m_pData->voxnum->ysiz;
  int iNumZ = m_pData->voxnum->zsiz;
  int iMinX = INT_MAX;
  int iMinY = INT_MAX;
  int iMinZ = INT_MAX;
  int iMaxX = INT_MIN;
  int iMaxY = INT_MIN;
  int iMaxZ = INT_MIN;
  if(bAccurate)
  {
    for(int x = 0; x < iNumX; x++)
    {
      int xoffset = x * iNumY;
      for(int y = 0; y < iNumY; y++)
      {
        if(m_pData->voxnum->ylen[xoffset + y] > 0)
        {
          if(x < iMinX)
            iMinX = x;
          if(x > iMaxX)
            iMaxX = x;
          if(y < iMinY)
            iMinY = y;
          if(y > iMaxY)
            iMaxY = y;
        }
      }
    }
    for(int i = 0; i < m_pData->voxnum->numvoxs; i++)
    {
      int z = m_pData->voxnum->vox[i].z;
      if(z < iMinZ)
        iMinZ = z;
      if(z > iMaxZ)
        iMaxZ = z;
    }
    iMaxX++;
    iMaxY++;
    iMaxZ++;
  }
  else
  {
    iMinX = 0;
    iMaxX = m_pData->voxnum->xsiz;
    iMinY = 0;
    iMaxY = m_pData->voxnum->ysiz;
    iMinZ = 0;
    iMaxZ = m_pData->voxnum->zsiz;
  }

  Vector3D vExtents((iMaxX - iMinX)*0.5f, (iMaxY - iMinY)*0.5f, (iMaxZ - iMinZ)*0.5f);
  Vector3D vCenter(iNumX - m_pData->voxnum->xpiv,m_pData->voxnum->ypiv,iNumZ - m_pData->voxnum->zpiv);
  vCenter -= vExtents;

  if(bTransformed)
  {
    Vector3D vExtentX(m_mTransform.m[0][0] * vExtents.x, m_mTransform.m[0][1] * vExtents.x, m_mTransform.m[0][2] * vExtents.x);
    vExtentX.x = fabsf(vExtentX.x);
    vExtentX.y = fabsf(vExtentX.y);
    vExtentX.z = fabsf(vExtentX.z);

    Vector3D vExtentY(m_mTransform.m[1][0] * vExtents.y, m_mTransform.m[1][1] * vExtents.y, m_mTransform.m[1][2] * vExtents.y);
    vExtentY.x = fabsf(vExtentY.x);
    vExtentY.y = fabsf(vExtentY.y);
    vExtentY.z = fabsf(vExtentY.z);

    Vector3D vExtentZ(m_mTransform.m[2][0] * vExtents.z, m_mTransform.m[2][1] * vExtents.z, m_mTransform.m[2][2] * vExtents.z);
    vExtentZ.x = fabsf(vExtentZ.x);
    vExtentZ.y = fabsf(vExtentZ.y);
    vExtentZ.z = fabsf(vExtentZ.z);

    vExtents.x = vExtentX.x + vExtentY.x + vExtentZ.x;
    vExtents.y = vExtentX.y + vExtentY.y + vExtentZ.y;
    vExtents.z = vExtentX.z + vExtentY.z + vExtentZ.z;

    vCenter = Transform(m_mTransform, vCenter);
  }

  vMin = vCenter - vExtents;
  vMax = vCenter + vExtents;
}

void Sprite::CreateUniqueData(void)
{
  bool bWasVisible = m_bVisible;
  // unregister
  if(bWasVisible)
    SetVisible(false);

  // clone sprite
  if(!m_bOwnedData)
  {
    vx5sprite *pOldData = m_pData;
    m_pData = new vx5sprite;
    memcpy(m_pData, pOldData, sizeof(vx5sprite));
  }
  m_bOwnedData = true;

  // clone voxel data
  kv6data *pOldVox = m_pData->voxnum;
  if(pOldVox != NULL)
  {
	  unsigned int iDataSize = pOldVox->numvoxs*sizeof(kv6voxtype) + pOldVox->xsiz*4 + pOldVox->xsiz*pOldVox->ysiz*2 + sizeof(kv6data);
    m_pData->voxnum = (kv6data *)malloc(iDataSize+sizeof(kv6data));
    memcpy(m_pData->voxnum, pOldVox, iDataSize+sizeof(kv6data));
	  m_pData->voxnum->namoff = 0;
	  m_pData->voxnum->lowermip = 0;
	  m_pData->voxnum->vox = (kv6voxtype *)(((long)m_pData->voxnum)+sizeof(kv6data));
	  m_pData->voxnum->xlen = (unsigned long *)(((long)m_pData->voxnum->vox)+m_pData->voxnum->numvoxs*sizeof(kv6voxtype));
	  m_pData->voxnum->ylen = (unsigned short *)(((long)m_pData->voxnum->xlen) + m_pData->voxnum->xsiz*4);
  }

  m_bUniqueData = true;

  // re-register
  if(bWasVisible)
    SetVisible(true);
}