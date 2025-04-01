#pragma once

class Sprite;
class ParticleSystem;

/// Base class for game objects
class Entity
{
public:
  /// All entity classes
  enum EntityClass
  {
    Class_Character,
    Class_Player,
    Class_Door,
    Class_Soldier,
    Class_Guard,
    Class_Officer,
    Class_Item,
    Class_Usable,
    Class_DummySprite
  };

  /// Collision types
  enum CollisionType
  {
    Collision_None = 0,
    Collision_Default = 1,
    Collision_Projectile = 2,
    Collision_Character = 4,
    Collision_Player = 8,
    Collision_Ragdoll = 16
  };

  /// Damage types
  enum DamageType
  {
    DamageType_Bullet = 0,
    DamageType_Explosion = 1,
    DamageType_Blade = 2,
    DamageType_HighExplosive = 3
  };

  /// Entity flags
  enum EntityFlag
  {
    /// Entity has changed position or size
    Flag_HasMoved = 1,
  };

public:
  Entity();
  virtual ~Entity();

  /// Updates the entity state
  virtual void Tick(void);

  /// Returns top most entity class
  inline const EntityClass &GetClass(void) { return m_Classes.back(); }
  /// Returns true if derives from class
  inline bool IsOfClass(int iClass) { for(unsigned int i = 0; i < m_Classes.size(); i++) if(m_Classes[i] == iClass) return true; return false; }

  /// Information about entity damage
  struct DamageInfo
  {
    DamageInfo() : Type(DamageType_Bullet), pHitSprite(NULL) {}
    /// Damage type
    DamageType Type;
    /// Amount of damage
    int iAmount;    
    /// Sprite that was hit
    Sprite *pHitSprite;
    /// Hit location
    Vector3D vHitPos;
    /// Hit normal
    Vector3D vHitNormal;
    /// Hit direction
    Vector3D vHitDir;
  };
  /// Entity was damaged
  virtual void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager) {}

  /// Sets entity position
  inline void SetPosition(const Vector3D &vNewPos) { m_vPosition = vNewPos; m_iFlags |= Flag_HasMoved; }
  /// Sets entity size
  inline void SetSize(const Vector3D &vNewSize) { m_vSize = vNewSize; m_iFlags |= Flag_HasMoved; }

  /// Marks that the entity position or AABB size has been changed
  inline void EntityMoved(void) { m_iFlags |= Flag_HasMoved; }

public:
  /// Performs collision detection against entities only, returns position where no collision occurs
  Vector3D ClipMove(const Vector3D &vPos, const Vector3D &vCollisionBox, const Vector3D &vMotion, unsigned int iCollisionTypeMask = -1);

  /// Results from entity hitscan
  struct HitInfo : public Voxlap::HitInfo
  {
    /// Hit entity or NULL if hit world
    Entity *pEntity;
    /// Sprite that was hit or NULL if none
    Sprite *pSprite;
  };
  /// Performs raytest against entities and world
  bool HitScan(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info, bool bTestWorld = true, unsigned int iCollisionTypeMask = -1);

  /// Performs raytest against this entity
  bool HitScanThis(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info);

public:
  /// Sends a sound event to notify enemies
  void SendSoundEvent(const Vector3D &vPos, float fRadius);

public:
  /// Position of the entity, when changed call EntityMoved()
  Vector3D m_vPosition;
  /// Euler angle rotation of the entity
  Vector3D m_vRotation;
  /// Size of bounding box that fully covers the entity, when changed call EntityMoved()
  Vector3D m_vSize;
  /// Velocity of the entity
  Vector3D m_vVelocity;

  /// Collision box size
  Vector3D m_vCollisionBox;
  /// Collision sprites for hitscans
  std::vector<Sprite *> m_CollisionSprites;
  
  /// Entity flags
  unsigned int m_iFlags;
  /// Collision type
  CollisionType m_CollisionType;

  /// Entity class hierarchy
  std::vector<EntityClass> m_Classes;

  /// Entity health
  int m_iHealth;

  /// Node in entity tree where this entity resides
  void *m_pTreeNode;

public:
  /// Increases reference count
  inline void AddReference(void) { m_iReferenceCount++; }
  /// Decreases reference count
  inline void RemoveReference(void) { m_iReferenceCount--; if(m_iReferenceCount == 0) delete this; }
  /// Reference count
  int m_iReferenceCount;
};

/// Applies generic damage to a sprite
extern void DamageSprite(Sprite *pSprite, const Entity::DamageInfo &Damage, bool bPlaySound = true);