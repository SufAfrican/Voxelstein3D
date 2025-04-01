#pragma once

#include "Game/Weapons/Weapon.h"

/// Grenade weapon
class Grenade : public Weapon
{
public:
  Grenade(Entity *pShooter);
  ~Grenade();

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void);
  /// Fires the weapon
  virtual void Fire(void);
  /// Animates the weapon
  virtual void Animate(void);

public:
  /// Grenade projectile
  struct Projectile
  {
    Sprite *pSprite;
    PhysicsSolver::Stick Stick;
    float fTime;
  };
  /// Thrown grenades
  std::vector<Projectile *> m_Grenades;
};
