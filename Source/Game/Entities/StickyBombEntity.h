#pragma once

class ExplosionCloudParticleSystem;

#include "Game/Entities/Entity.h"

/// A slab of C4 ticking and going off
class StickyBombEntity : public Entity
{
public:
  StickyBombEntity(Entity *pShooter, const Vector3D &vPos, const Vector3D &vNormal);
  ~StickyBombEntity();

  /// Updates the entity state
  virtual void Tick(void);

  /// Blows the thing up
  void Detonate(void);

private:
  /// Updates explosion clouds
  void TickExplosions(void);

private:
  /// Entity which created the bomb
  SmartPointer<Entity> m_pShooter;
  /// Normal of surface where the bomb was placed
  Vector3D m_vNormal;
  /// Bomb sprite
  Sprite *m_pSprite;
  /// Time since creation
  unsigned int m_iTime;
  /// Has been detonated
  bool m_bDetonated;
  /// A single cloud of explosion
  struct ExplosionNode
  {
    Vector3D vPos;
    Vector3D vDir;
    float fRadius;
    float fEnergy;
    ExplosionCloudParticleSystem *pParticle;
  };
  /// Active explosions
  std::vector<ExplosionNode> m_Explosions;
};