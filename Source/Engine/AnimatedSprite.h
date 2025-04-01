#pragma once

class AnimationFile;

/// A sprite that can be posed by animation
class SpritePoser
{
public:
  SpritePoser(Sprite *pSprite);
  ~SpritePoser();

  /// Updates sprite
  void UpdateSprite(const Matrix44 &mTransform);
  /// Returns sprite
  inline Sprite *GetSprite(void) { return m_pSprite; }

public:
  /// Sprite
  Sprite *m_pSprite;
  /// Position
  Vector3D m_vPosition;
  /// Rotation
  Quaternion m_qRotation;
  /// Scale
  Vector3D m_vScale;
};

/// Instance for playing back AnimationFiles
class AnimatedSprite
{
public:
  AnimatedSprite(AnimationFile *pFile);
  ~AnimatedSprite();

  /// Updates sprites from current animation
  void Run(float fDeltaTime, bool bApplyAnimation = true);

  /// Plays animation
  inline void PlayAnim(int iAnim, bool bLooping)
  {
    if(m_iCurrentAnim != iAnim)
    {
      // start over
      m_bPlaying = true;
      m_fCurrentTime = 0.0f;
      m_bLooping = bLooping;     
      m_iCurrentAnim = iAnim;
    }    
  }

  /// Sets transformation matrix
  inline void SetTransform(const Matrix44 &mTrans) { m_mTransform = mTrans; }
  /// Returns transformation matrix
  inline const Matrix44 &GetTransform(void) { return m_mTransform; }

  /// Returns all sprites
  inline void ListSprites(std::vector<Sprite *> &Sprites)
  {
    Sprites.resize(m_Sprites.size());
    for(unsigned int i = 0; i < m_Sprites.size(); i++)
      Sprites[i] = m_Sprites[i]->GetSprite();
  }

  /// Calculates sprite size, transformed or not
  void CalculateBoundingBox(Vector3D &vMin, Vector3D &vMax, bool bTransformed = true, bool bAccurate = false);

  /// Finds a sprite poser with given sprite filename
  SpritePoser *FindSprite(const char *strSprite);

  /// Puts sprite into ragdoll mode
  void CreateRagdoll(float fScale = 1.0f);
  /// Puts sprite back into animated mode
  void DestroyRagdoll(void);

  /// Adds impulse to ragdoll
  void AddImpulse(const Vector3D &vPos, const Vector3D &vImpulse);
  /// Adds radial impulse to ragdoll
  void AddRadialImpulse(const Vector3D &vPos, float fImpulse, float fRadius);

  /// Returns true if sprite is in ragdoll mode
  inline bool IsRagdoll(void) { return m_bRagdoll; }

private:
  /// Updates poses from ragdoll state
  void UpdateFromRagdoll(void);
  /// Updates ragdoll transforms
  void UpdateRagdollTransforms(void);
  /// Returns index to m_Particles
  unsigned int GetParticleIndex(unsigned int iSpritePhysics, unsigned int iParticle);

public:
  /// Sprites used by the animation
  std::vector<SpritePoser *> m_Sprites;
  /// Current animation index
  int m_iCurrentAnim;
  /// Time in current animation
  float m_fCurrentTime;
  /// Is animation looping
  bool m_bLooping;
  /// Is animation playing
  bool m_bPlaying;
  /// Animation file
  AnimationFile *m_pFile;
  /// Transformation
  Matrix44 m_mTransform;

  /// Data for a sprite in ragdoll mode
  struct RagdollSprite
  {
    unsigned int iSprite;
    std::vector<unsigned int> MovePointParticles;
    Matrix44 mTransform;
    Matrix44 mInitialInverse;
    Vector3D vFront;
    Quaternion qRot;
  };
  /// Sprites in ragdoll mode
  std::vector<RagdollSprite> m_RagdollSprites;
  /// Ragdoll particles
  std::vector<unsigned int> m_Particles;
  /// Ragdoll constraints
  std::vector<unsigned int> m_Constraints;

private:
  /// Is using ragdoll
  bool m_bRagdoll;
};