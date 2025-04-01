#pragma once

class SpritePoser;
class AnimatedSprite;

/// File that holds animation data
class AnimationFile
{
public:
  /// Adds a new sprite, updates instance
  bool AddSprite(const char *strFilename, AnimatedSprite *pInstance);
  /// Deletes a sprite, updates instance
  void DeleteSprite(unsigned int iSprite, AnimatedSprite *pInstance);
  /// Returns sprite index
  unsigned int GetSprite(const std::string &strFile);

  /// Adds a new animation, returns index
  unsigned int AddAnimation(const std::string &strName, float fDuration);
  /// Removes an animation
  void RemoveAnimation(unsigned int iAnim);
  /// Returns animation index
  unsigned int GetAnimation(const std::string &strName);

  /// Adds a new frame, returns index
  unsigned int AddFrame(unsigned int iAnim, float fTime, float fTolerance);
  /// Removes a frame
  void RemoveFrame(unsigned int iAnim, unsigned int iFrame);
  /// Returns frame index
  unsigned int GetFrame(unsigned int iAnim, float fTime, float fTolerance);
  /// Resorts frame array in an animation
  void SortFrames(unsigned int iAnim);

  /// Copies poses from sprites to given frame
  void SetPoses(unsigned int iAnim, unsigned int iFrame, const std::vector<SpritePoser *> &Sprites);

  /// Copies a partial set of poses from sprites to given frame
  void SetPoses(unsigned int iAnim, unsigned int iFrame, const std::vector<unsigned int> &SpriteIndices, const std::vector<SpritePoser *> &Sprites);

  /// Loads animation from file
  bool LoadFrom(const char *strFile);
  /// Saves animation to file
  bool SaveTo(const char *strFile);

  /// Creates a new instance that uses this file
  AnimatedSprite *CreateNewInstance(void);

  /// Returns index to physics for given sprite, or -1
  unsigned int GetSpritePhysicsIndex(unsigned int iSprite);
  /// Adds sprite physics for given sprite or returns existing one
  unsigned int AddSpritePhysics(unsigned int iSprite);

  /// Removes given particle
  void RemoveParticle(unsigned int iSpritePhysics, unsigned int iParticle);
  /// Removes given sprite physics
  void RemoveSpritePhysics(unsigned int iSpritePhysics);
  /// Removes sprite physics that are not used
  void RemoveEmptySpritePhysics(void);
  /// Changes spritephysics index of given particle, returns new particle index
  unsigned int ChangeParticleSpritePhysics(unsigned int iSpritePhysics, unsigned int iParticle, unsigned int iNewSpritePhysics);

  /// Tests if something has gone terribly wrong with physics data
  bool ValidatePhysics(void);
public:
  /// State of a sprite in a frame
  struct SpritePose
  {
    Vector3D vPosition;
    Quaternion qRotation;
    Vector3D vScale;
  };

  // A single animation
  struct Animation
  {
    /// A single frame in animation
    struct Frame
    {
      /// One pose for each sprite in m_Sprites
      std::vector<SpritePose> Poses;
      /// Normalized time 0..1
      float fTime;
    };

    /// Frames in animation, sorted by time
    std::vector<Frame> Frames;
    /// Duration of animation
    float fDuration;
    /// Name of the animation
    std::string strName;
  };

  /// Animations
  std::vector<Animation> m_Animations;
  /// Sprites used by animation
  std::vector<std::string> m_SpriteFiles;


  /// Ragdoll particle
  struct PhysicsParticle
  {
    Vector3D vPosition;
    float fRadius;
  };

  /// Index to a particle
  struct ParticleIndex
  {
    ParticleIndex() : iSpritePhysics(-1), iParticle(-1) {}
    ParticleIndex(unsigned int iSpritePhysics, unsigned int iParticle) : iSpritePhysics(iSpritePhysics), iParticle(iParticle) {}
    
    /// Index to m_SpritePhysics
    unsigned int iSpritePhysics;
    /// Index to SpritePhysics::Particles
    unsigned int iParticle;

    /// Returns true if index should point to something
    inline bool IsSet(void) { return iSpritePhysics != -1 && iParticle != -1; }
    /// Compares two indices
    inline bool operator == (const ParticleIndex &other) { return (iSpritePhysics == other.iSpritePhysics && iParticle == other.iParticle); }
    /// Compares two indices
    inline bool operator != (const ParticleIndex &other) { return (iSpritePhysics != other.iSpritePhysics || iParticle != other.iParticle); }
  };

  /// Ragdoll particle constraint
  struct PhysicsConstraint
  {
    /// Index to SpritePhysics
    unsigned int iSpritePhysics1, iSpritePhysics2;
    /// Index to PhysicsParticle of SpritePhysics
    unsigned int iParticle1, iParticle2;
    /// Constraint distance or 0.0f for default
    float fDistance;

    /// Compares if constraints are connected to same particles
    inline bool ConnectedToSame(const PhysicsConstraint &other)
    {
      if(iSpritePhysics1 == other.iSpritePhysics1 && iParticle1 == other.iParticle1)
      {
        return iSpritePhysics2 == other.iSpritePhysics2 && iParticle2 == other.iParticle2;
      }
      else if(iSpritePhysics2 == other.iSpritePhysics1 && iParticle2 == other.iParticle1)
      {
        return iSpritePhysics1 == other.iSpritePhysics2 && iParticle1 == other.iParticle2;
      }
      return false;
    }
  };


  /// Sprite physics setup for ragdoll
  struct SpritePhysics
  {
    /// Sprite index
    unsigned int iSprite;
    /// Particles relative to sprite
    std::vector<PhysicsParticle> Particles;
    /// Particles that determine coordinate system, may be from another sprite
    ParticleIndex MovePointParticles[3];
  };

  /// Ragdoll physics for each sprite
  std::vector<SpritePhysics> m_SpritePhysics;
  /// Constraints
  std::vector<PhysicsConstraint> m_PhysicsConstraints;
};

/// Manager that helps loading only one instance of each animation file
class AnimationFileManager
{
private:
  AnimationFileManager();
  ~AnimationFileManager();

public:
  /// Loads and manages file, returns NULL if loading fails
  AnimationFile *LoadFile(const char *strFile);
  /// Creates sprite from file, returns NULL if loading fails
  AnimatedSprite *CreateFromFile(const char *strFile);

  /// Returns singleton instance
  static AnimationFileManager *GetInstance(void);
private:
  std::map<std::string, AnimationFile *> m_Files;
};
