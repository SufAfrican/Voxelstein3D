#pragma once

class Sprite;

/// Image data as loaded by kplib
struct VoxlapImage
{
  long pData;
  long iBytesPerLine;
  long iWidth;
  long iHeight;
};

/// Simple wrapper functions for voxlap
namespace Voxlap
{
  /// Returns floor height at given position
  float GetFloorHeight(const Vector3D &vPos);
  /// Returns normal at given position
  Vector3D GetNormal(const Vector3D &vPos);
  /// Does a sweep test, returns position where no collision occurs
  Vector3D ClipMove(const Vector3D &vPos, const Vector3D &vMotion, float fRadius);

  /// Hitscan result info
  struct HitInfo
  {
    /// Hit position
    Vector3D vPos;
    /// Hit normal
    Vector3D vNormal;
    /// Hit voxel color
    int iColor;
    /// Hit distance along ray
    float fDistance;
  };

  /// Performs a raytest, returns true if hit
  bool HitScan(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info);
  /// Performs a raytest against sprite, returns true if hit
  bool HitScan(Sprite *pSprite, const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info);

  /// Updates a voxels color
  void SetVoxelColor(const Vector3D &vPos, int iR, int iG, int iB);
  /// Updates a voxels color in a sprite
  void SetVoxelColor(Sprite *pSprite, const Vector3D &vPos, int iR, int iG, int iB);
  /// Updates a voxels color in a sprite
  void SetVoxelColor(Sprite *pSprite, int x, int y, int z, int iR, int iG, int iB);
  /// Fills a voxel sphere with color
  void SetVoxelSphere(const Vector3D &vPos, int iRadius, int iR, int iG, int iB);
  /// Places a bullet hole in the voxels
  void DrawVoxelBulletHole(const Vector3D &vPos);
  /// Places a bullet hole in sprite
  void DrawVoxelBulletHole(Sprite *pSprite, const Vector3D &vPos);
  /// Places a big bullet hole in the voxels
  void DrawVoxelBulletHoleBig(const Vector3D &vPos);
  /// Replaces voxel with empty space
  void ClearVoxel(const Vector3D &vPos);
  /// Replaces voxels in sphere with empty space
  void ClearVoxelSphere(const Vector3D &vPos, int iRadius);
  /// Replaces voxel with empty space in a sprite
  //void ClearVoxel(Sprite *pSprite, const Vector3D &vPos);
  /// Places a voxel explosion hole in the voxels
  void DrawVoxelExplosionHole(const Vector3D &vPos, int iRadius);
  /// Places a voxel wound in sprite
  void DrawVoxelWound(Sprite *pSprite, const Vector3D &vPos, int iHoleRadius, int iBloodRadius);
  /// Places a blood splat hole in the voxels
  void DrawVoxelBloodSplat(const Vector3D &vPos, unsigned char iStrength);

  /// Colorizes screen for given duration
  void SetColorFilter(int iR, int iG, int iB, float fDuration);

  /// TEMP: Flash effect
  void ShowFlashEffect(const Vector3D &vPos, int iRadius, int iIntensity=2048);
  /// TEMP: Draws a debug line
  void AddDebugLine(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bPersistent = false);
  /// TEMP: Draws a debug box
  void AddDebugBoxLines(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bPersistent = false);

  /// Returns pointer to loaded image (managed)
  VoxlapImage *LoadImage(const char *strFile);
}