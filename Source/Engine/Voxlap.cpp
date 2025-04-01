#include "StdAfx.h"

#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

struct DebugLine
{
  point3d vStart, vEnd;
  int iR,iG,iB;
  bool bPersistent;
};
std::vector<DebugLine> g_DebugLines;

int g_iColorFilterR = 0;
int g_iColorFilterG = 0;
int g_iColorFilterB = 0;
float g_fColorFilterTime = 0;

namespace Voxlap
{
  float GetFloorHeight(const Vector3D &vPos)
  {
    // BROKEN!!
    //return getfloorz(-vPos.x, vPos.z, vPos.y);

    dpoint3d pos = ConvertToVoxlap<dpoint3d>(vPos);
    dpoint3d dir = {0,0,1};
    lpoint3d hit = {0,0,0};
    long *index = 0;
    long hdir = 0;
    hitscan(&pos, &dir, &hit, &index, &hdir);
    if(index != 0)
    {
      return ConvertFromVoxlap(hit).y;
    }
    return vPos.y;
  }

  Vector3D GetNormal(const Vector3D &vPos)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    point3d norm;
    estnorm(pos.x, pos.y, pos.z, &norm);
    return -ConvertFromVoxlap<point3d>(norm);
  }

  Vector3D ClipMove(const Vector3D &vPos, const Vector3D &vMotion, float fRadius)
  {
    dpoint3d pos = ConvertToVoxlap<dpoint3d>(vPos);
    dpoint3d mot = ConvertToVoxlap<dpoint3d>(vMotion);
    clipmove(&pos, &mot, fRadius);
    return ConvertFromVoxlap(pos);
  }

  bool HitScan(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info)
  {
    dpoint3d pos = ConvertToVoxlap<dpoint3d>(vPos);
    dpoint3d dir = ConvertToVoxlap<dpoint3d>(vDir);
    lpoint3d hit;
    long *color = 0;
    long hitdir = 0;
    hitscan(&pos, &dir, &hit, &color, &hitdir);
    if(color != 0)
    {
      Vector3D vHitPos = ConvertFromVoxlap(hit);
      float fHitDistance = (vHitPos - vPos).Length();

      // hit too far
      if(fHitDistance > fDistance)
        return false;

      info.fDistance = fHitDistance;
      info.vPos = vHitPos;
      info.iColor = *(int *)color;

      switch(hitdir)
      {
      case 0:
        info.vNormal = Vector3D(1,0,0);
        break;
      case 1:
        info.vNormal = Vector3D(-1,0,0);
        break;
      case 2:
        info.vNormal = Vector3D(0,0,-1);
        break;
      case 3:
        info.vNormal = Vector3D(0,0,1);
        break;
      case 4:
        info.vNormal = Vector3D(0,1,0);
        break;
      default:
        info.vNormal = Vector3D(0,-1,0);
        break;
      }
      return true;
    }
    return false;
  }

  bool HitScan(Sprite *pSprite, const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info)
  {
    if(pSprite->GetData()->voxnum == NULL)
      return false;

    lpoint3d hit;
    kv6voxtype *hitvox = 0;
    long hitdir = 0;

    dpoint3d pos = ConvertToVoxlap<dpoint3d>(vPos);
    dpoint3d dir = ConvertToVoxlap<dpoint3d>(vDir);
    sprhitscan(&pos, &dir, pSprite->GetData(), &hit, &hitvox, &hitdir, &fDistance);

    if(hitvox != 0)
    {
      // transform point
      Vector3D vPivot(pSprite->GetData()->voxnum->xpiv, pSprite->GetData()->voxnum->ypiv, pSprite->GetData()->voxnum->zpiv);
      Vector3D vHit = MakeVector<lpoint3d>(hit) - vPivot;

      Matrix44 mSprite(pSprite->GetData()->s.x, pSprite->GetData()->s.y, pSprite->GetData()->s.z, 0,
                       pSprite->GetData()->h.x, pSprite->GetData()->h.y, pSprite->GetData()->h.z, 0,
                       pSprite->GetData()->f.x, pSprite->GetData()->f.y, pSprite->GetData()->f.z, 0,
                       pSprite->GetData()->p.x, pSprite->GetData()->p.y, pSprite->GetData()->p.z, 1);
  
      Vector3D vHitTransformed = Transform(mSprite, vHit);
      info.vPos = ConvertFromVoxlap(vHitTransformed);
      info.iColor = hitvox->col;
      info.fDistance = (info.vPos - vPos).Length();

      switch(hitdir)
      {
      case 0:
        info.vNormal = Vector3D(1,0,0);
        break;
      case 1:
        info.vNormal = Vector3D(-1,0,0);
        break;
      case 2:
        info.vNormal = Vector3D(0,0,-1);
        break;
      case 3:
        info.vNormal = Vector3D(0,0,1);
        break;
      case 4:
        info.vNormal = Vector3D(0,1,0);
        break;
      default:
        info.vNormal = Vector3D(0,-1,0);
        break;
      }
      //if(hitvox->dir >= 0 && hitvox->dir < 255)
      //  equiind2vec(hitvox->dir, &info.vNormal.x, &info.vNormal.y, &info.vNormal.z);
      info.vNormal = Normalize(Transform3x3(pSprite->GetTransform(), ConvertFromVoxlap(info.vNormal)));
      return true;
    }
    return false;
  }

  void SetVoxelColor(const Vector3D &vPos, int iR, int iG, int iB)
  {
    long color = 0x80000000 | ((iR&0xFF)<<16) | ((iG&0xFF)<<8) | (iB&0xFF);//0x80RRGGBB;
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    setcube(pos.x, pos.y, pos.z, color);
  }

  void SetVoxelColor(Sprite *pSprite, int x, int y, int z, int iR, int iG, int iB)
  {
    long color = 0x80000000 | ((iR&0xFF)<<16) | ((iG&0xFF)<<8) | (iB&0xFF);//0x80RRGGBB;
    sprcolorizecube(pSprite->GetData(), x, y, z, color);
  }

  void SetVoxelColor(Sprite *pSprite, const Vector3D &vPos, int iR, int iG, int iB)
  {
    if(pSprite->GetData()->voxnum == NULL)
      return;
    if(!pSprite->IsUniqueData())
      pSprite->CreateUniqueData();

    Matrix44 mSprite(pSprite->GetData()->s.x, pSprite->GetData()->s.y, pSprite->GetData()->s.z, 0,
                     pSprite->GetData()->h.x, pSprite->GetData()->h.y, pSprite->GetData()->h.z, 0,
                     pSprite->GetData()->f.x, pSprite->GetData()->f.y, pSprite->GetData()->f.z, 0,
                     pSprite->GetData()->p.x, pSprite->GetData()->p.y, pSprite->GetData()->p.z, 1);

    Matrix44 mSpriteInv = Inverse(mSprite);
    Vector3D vPosRel = Transform(mSpriteInv, ConvertToVoxlap<Vector3D>(vPos));
    Vector3D vPivot(pSprite->GetData()->voxnum->xpiv, pSprite->GetData()->voxnum->ypiv, pSprite->GetData()->voxnum->zpiv);
    vPosRel += vPivot;

    // round to nearest due to precision error
    if(vPosRel.x > 0)
      vPosRel.x += 0.5f;
    if(vPosRel.y > 0)
      vPosRel.y += 0.5f;
    if(vPosRel.z > 0)
      vPosRel.z += 0.5f;

    SetVoxelColor(pSprite, vPosRel.x, vPosRel.y, vPosRel.z, iR, iG, iB);
  }

  void DrawVoxelWound(Sprite *pSprite, const Vector3D &vPos, int iHoleRadius, int iBloodRadius)
  {
    if(pSprite->GetData()->voxnum == NULL)
      return;
    if(!pSprite->IsUniqueData())
      pSprite->CreateUniqueData();

    Matrix44 mSprite(pSprite->GetData()->s.x, pSprite->GetData()->s.y, pSprite->GetData()->s.z, 0,
                     pSprite->GetData()->h.x, pSprite->GetData()->h.y, pSprite->GetData()->h.z, 0,
                     pSprite->GetData()->f.x, pSprite->GetData()->f.y, pSprite->GetData()->f.z, 0,
                     pSprite->GetData()->p.x, pSprite->GetData()->p.y, pSprite->GetData()->p.z, 1);

    Matrix44 mSpriteInv = Inverse(mSprite);
    Vector3D vPosRel = Transform(mSpriteInv, ConvertToVoxlap<Vector3D>(vPos));
    Vector3D vPivot(pSprite->GetData()->voxnum->xpiv, pSprite->GetData()->voxnum->ypiv, pSprite->GetData()->voxnum->zpiv);
    vPosRel += vPivot;

    // round to nearest due to precision error
    if(vPosRel.x > 0)
      vPosRel.x += 0.5f;
    if(vPosRel.y > 0)
      vPosRel.y += 0.5f;
    if(vPosRel.z > 0)
      vPosRel.z += 0.5f;

    sprdrawwound(pSprite->GetData(), vPosRel.x, vPosRel.y, vPosRel.z, iHoleRadius, iBloodRadius);
  }

  void DrawVoxelBulletHole(Sprite *pSprite, const Vector3D &vPos)
  {
    if(pSprite->GetData()->voxnum == NULL)
      return;
    if(!pSprite->IsUniqueData())
      pSprite->CreateUniqueData();

    Matrix44 mSprite(pSprite->GetData()->s.x, pSprite->GetData()->s.y, pSprite->GetData()->s.z, 0,
                     pSprite->GetData()->h.x, pSprite->GetData()->h.y, pSprite->GetData()->h.z, 0,
                     pSprite->GetData()->f.x, pSprite->GetData()->f.y, pSprite->GetData()->f.z, 0,
                     pSprite->GetData()->p.x, pSprite->GetData()->p.y, pSprite->GetData()->p.z, 1);

    Matrix44 mSpriteInv = Inverse(mSprite);
    Vector3D vPosRel = Transform(mSpriteInv, ConvertToVoxlap<Vector3D>(vPos));
    Vector3D vPivot(pSprite->GetData()->voxnum->xpiv, pSprite->GetData()->voxnum->ypiv, pSprite->GetData()->voxnum->zpiv);
    vPosRel += vPivot;

    // round to nearest due to precision error
    if(vPosRel.x > 0)
      vPosRel.x += 0.5f;
    if(vPosRel.y > 0)
      vPosRel.y += 0.5f;
    if(vPosRel.z > 0)
      vPosRel.z += 0.5f;

    sprdrawbullethole(pSprite->GetData(), vPosRel.x, vPosRel.y, vPosRel.z);
  }

  void ClearVoxel(const Vector3D &vPos)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    setcube(pos.x, pos.y, pos.z, -1);
  }

// VS2005 compiler bug: does not make the actual hole
#pragma optimize("", off)
  void DrawVoxelBulletHole(const Vector3D &vPos)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);

    // crap hack: make wood hole
    if(pos.x >= 384 && pos.x <= 615
      && pos.y >= 738 && pos.y <= 824
      && pos.z >= 170 && pos.z <= 255)
    {
      for(int i = 0; i < 3; i++)
      {
        int iHeight = 3 + rand()%5;
        clearcolumn(pos.x + i, pos.y, pos.z - iHeight, iHeight+iHeight);
        iHeight = 3 + rand()%5;
        clearcolumn(pos.x, pos.y + i, pos.z - iHeight, iHeight+iHeight);
      }
      return;
    }

    // make actual hole
    vx5.curcol = 0;
    setcube(pos.x, pos.y, pos.z, -1);

    // black star shape
    static const lpoint3d offsets[6] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
    for(int i = 0; i < 6; i++)
    {
      long pColor = getcube(pos.x + offsets[i].x,
                            pos.y + offsets[i].y,
                            pos.z + offsets[i].z);
      if(pColor == 1)
      {
        setcube(pos.x + offsets[i].x,
                pos.y + offsets[i].y,
                pos.z + offsets[i].z, 0x00000000);
      }
      else if(pColor > 1)
      {
        long newcolor = *(long *)pColor;
        int intensity = (newcolor>>24) & 0xFF;
        // mul by 0.8
        intensity *= 8; intensity /= 10;
        newcolor = (newcolor & 0x00FFFFFF) | (intensity<<24);

        setcube(pos.x + offsets[i].x,
                pos.y + offsets[i].y,
                pos.z + offsets[i].z, newcolor);
      }
    }
  }
  
  void DrawVoxelBloodSplat(const Vector3D &vPos, unsigned char iStrength)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);

    int iNewFactor = iStrength;
    int iOldFactor = 256 - iNewFactor;

    int iNewFactor_Soft = iStrength/4;
    int iOldFactor_Soft = 256 - iNewFactor_Soft;

    int iRed = 50 + rand()%50;

    long pColor = getcube(pos.x, pos.y, pos.z);
    if(pColor > 1)
    {
      long newcolor = *(long *)pColor;
      int a = (newcolor >> 24) & 0xFF;
      int r = (newcolor >> 16) & 0xFF;
      int g = (newcolor >> 8) & 0xFF;
      int b = (newcolor) & 0xFF;
      // mul
      r = (r*iOldFactor + iRed*iNewFactor) / 256;
      g *= iOldFactor; g /= 256;
      b *= iOldFactor; b /= 256;
      newcolor = (a << 24) | (r << 16) | (g << 8) | b;

      setcube(pos.x, pos.y, pos.z, newcolor);
    }

    // black star shape
    static const lpoint3d offsets[6] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
    for(int i = 0; i < 6; i++)
    {
      long pColor = getcube(pos.x + offsets[i].x,
                            pos.y + offsets[i].y,
                            pos.z + offsets[i].z);
      if(pColor > 1)
      {
        long newcolor = *(long *)pColor;
        int a = (newcolor >> 24) & 0xFF;
        int r = (newcolor >> 16) & 0xFF;
        int g = (newcolor >> 8) & 0xFF;
        int b = (newcolor) & 0xFF;
        r = (r*iOldFactor_Soft + iRed*iNewFactor_Soft) / 256;
        g *= iOldFactor_Soft; g /= 256;
        b *= iOldFactor_Soft; b /= 256;
        newcolor = (a << 24) | (r << 16) | (g << 8) | b;

        setcube(pos.x + offsets[i].x,
                pos.y + offsets[i].y,
                pos.z + offsets[i].z, newcolor);
      }
    }
  }

  void DrawVoxelBulletHoleBig(const Vector3D &vPos)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);

    // crap hack: make wood hole
    if(pos.x >= 384 && pos.x <= 615
      && pos.y >= 738 && pos.y <= 824
      && pos.z >= 170 && pos.z <= 255)
    {
      for(int i = 0; i < 3; i++)
      {
        int iHeight = 4 + rand()%6;
        clearcolumn(pos.x + i, pos.y, pos.z - iHeight, iHeight+iHeight);
        iHeight = 4 + rand()%6;
        clearcolumn(pos.x, pos.y + i, pos.z - iHeight, iHeight+iHeight);
      }
    }

    // make actual hole
    vx5.curcol = 0;
    setcube(pos.x, pos.y, pos.z, -1);

    // black star shape
    {
      static const lpoint3d offsets[6] = {{1,0,0}, {-1,0,0}, {0,1,0}, {0,-1,0}, {0,0,1}, {0,0,-1}};
      for(int i = 0; i < 6; i++)
      {
        long pColor = getcube(pos.x + offsets[i].x,
                              pos.y + offsets[i].y,
                              pos.z + offsets[i].z);
        if(pColor == 1)
        {
          setcube(pos.x + offsets[i].x,
                  pos.y + offsets[i].y,
                  pos.z + offsets[i].z, 0x00000000);
        }
        else if(pColor > 1)
        {
          long newcolor = *(long *)pColor;
          int intensity = (newcolor>>24) & 0xFF;
          // mul by 0.7
          intensity *= 7; intensity /= 10;
          newcolor = (newcolor & 0x00FFFFFF) | (intensity<<24);

          setcube(pos.x + offsets[i].x,
                  pos.y + offsets[i].y,
                  pos.z + offsets[i].z, newcolor);
        }
      }
    }

    // black diamond surroundings
    {
      static const lpoint3d offsets[18] = {{2,0,0},{-2,0,0},
                                           {0,2,0},{0,-2,0},
                                           {0,0,2},{0,0,-2},
                                           {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
                                           {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
                                           {0,1,1},{0,1,-1},{0,-1,1},{0,-1,-1}};
      for(int i = 0; i < 18; i++)
      {
        long pColor = getcube(pos.x + offsets[i].x,
                              pos.y + offsets[i].y,
                              pos.z + offsets[i].z);
        if(pColor == 1)
        {
          setcube(pos.x + offsets[i].x,
                  pos.y + offsets[i].y,
                  pos.z + offsets[i].z, 0x00000000);
        }
        else if(pColor > 1)
        {
          long newcolor = *(long *)pColor;
          int intensity = (newcolor>>24) & 0xFF;
          // mul by 0.8
          intensity *= 8; intensity /= 10;
          newcolor = (newcolor & 0x00FFFFFF) | (intensity<<24);

          setcube(pos.x + offsets[i].x,
                  pos.y + offsets[i].y,
                  pos.z + offsets[i].z, newcolor);
        }
      }
    }
  }
#pragma optimize("", on)

  void SetVoxelSphere(const Vector3D &vPos, int iRadius, int iR, int iG, int iB)
  {
    long color = 0xFF000000 | ((iR&0xFF)<<16) | ((iG&0xFF)<<8) | (iB&0xFF);//0x80RRGGBB;
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    long oldcol = vx5.curcol;
    vx5.curcol = color;
    vx5.colfunc = curcolfunc;
    setsphere(&pos, iRadius, 0);
    vx5.curcol = oldcol;
  }

  void ClearVoxelSphere(const Vector3D &vPos, int iRadius)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    setsphere(&pos, iRadius, -1);
  }

  void DrawVoxelExplosionHole(const Vector3D &vPos, int iRadius)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    vx5.curcol = 0;
    pos = ConvertToVoxlap<lpoint3d>(vPos);
    setnormflash(pos.x, pos.y, pos.z, iRadius*2, -1024);
  }

  void ShowFlashEffect(const Vector3D &vPos, int iRadius, int iIntensity)
  {
    lpoint3d pos = ConvertToVoxlap<lpoint3d>(vPos);
    setnormflash(pos.x, pos.y, pos.z, iRadius, iIntensity);
  }

  void PlaySound(const std::string &strSound, const Vector3D &vPos, float fFreq, int iVolume)
  {
    //point3d pos = ConvertToVoxlap<point3d>(vPos);
    //playsound(strSound.c_str(),iVolume,fFreq,&pos,KSND_3D);
  }

  void AddDebugLine(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bPersistent)
  {
    DebugLine line = {ConvertToVoxlap<point3d>(vStart),ConvertToVoxlap<point3d>(vEnd),iR,iG,iB,bPersistent};
    g_DebugLines.push_back(line);
  }

  void AddDebugBoxLines(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bPersistent)
  {
    Vector3D vSize = vEnd - vStart;
    AddDebugLine(vStart, vStart + Vector3D(vSize.x,0,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart, vStart + Vector3D(0,vSize.y,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart, vStart + Vector3D(0,0,vSize.z), iR,iG,iB,bPersistent);

    AddDebugLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(vSize.x,0,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(0,vSize.y,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(vSize.x,vSize.y,0), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bPersistent);

    AddDebugLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(0,0,vSize.z), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(0,vSize.y,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(0,vSize.y,vSize.z), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bPersistent);

    AddDebugLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(0,0,vSize.z), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(vSize.x,0,0), iR,iG,iB,bPersistent);
    AddDebugLine(vStart + Vector3D(vSize.x,0,vSize.z), vStart + Vector3D(vSize.x,vSize.y,vSize.z), iR,iG,iB,bPersistent);
  }

  void SetColorFilter(int iR, int iG, int iB, float fDuration)
  {
    g_iColorFilterR = iR;
    g_iColorFilterG = iG;
    g_iColorFilterB = iB;
    g_fColorFilterTime += fDuration;
  }

  class VoxlapImageLoader
  {
  public:
    VoxlapImageLoader::~VoxlapImageLoader()
    {
      for(ImageMap::iterator it = m_Images.begin(); it != m_Images.end(); it++)
      {
        free((void*)it->second->pData);
        delete it->second;
      }
    }

    VoxlapImage *LoadImage(const char *strFile)
    {
      ImageMap::iterator it = m_Images.find(strFile);
      if(it != m_Images.end())
        return it->second;

      VoxlapImage *pImg = new VoxlapImage;
      kpzload(strFile, &pImg->pData, &pImg->iBytesPerLine, &pImg->iWidth, &pImg->iHeight);
      m_Images[strFile] = pImg;
      return pImg;
    }

    typedef std::map<std::string, VoxlapImage *> ImageMap;
    ImageMap m_Images;
  };

  VoxlapImage *LoadImage(const char *strFile)
  {
    static VoxlapImageLoader loader;
    return loader.LoadImage(strFile);
  }
}