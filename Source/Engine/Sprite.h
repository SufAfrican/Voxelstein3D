#pragma once

struct vx5sprite;

/// A voxel sprite (wrapper for vx5sprite)
class Sprite
{
public:
  /// Loads a new sprite from file
  Sprite(const std::string &strFile, bool bRenderInFront = false);
  /// Creates a sprite with existing data
  Sprite(vx5sprite *pData, bool bRenderInFront = false);
  ~Sprite();

  /// Changes the transformation matrix
  void SetTransform(const Matrix44 &mTrans);
  /// Returns transformation matrix
  inline const Matrix44 &GetTransform(void) { return m_mTransform; }

  /// Set sprite visibility
  void SetVisible(bool bVisible);
  /// Changes whether sprite is rendered in front of all others
  void SetRenderInFront(bool bRenderInFront);

  /// Calculates sprite size, transformed or not
  void CalculateBoundingBox(Vector3D &vMin, Vector3D &vMax, bool bTransformed = true, bool bAccurate = false);

  /// Returns filename of the sprite
  inline const std::string &GetFilename(void) { return m_strFilename; }
  /// Returns true if data is unique to this instance
  inline bool IsUniqueData(void) { return m_bUniqueData; }
  /// Returns sprite data
  inline vx5sprite *GetData(void) { return m_pData; }
  /// Creates an unique copy of data (for modifying the voxels of this instance)
  void CreateUniqueData(void);

private:
  /// Rotation and translation matrix
  Matrix44 m_mTransform;
  /// Filename of the sprite
  std::string m_strFilename;
  /// Voxlap data
  vx5sprite *m_pData;
  /// Should the data be deleted once the sprite is deleted
  bool m_bOwnedData;
  /// Has the data content been altered making it unique for this sprite
  bool m_bUniqueData;
  /// Is sprite rendered in front of everything
  bool m_bRenderInFront;
  /// Is sprite visible
  bool m_bVisible;
};