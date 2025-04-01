#pragma once

#include "Game/Weapons/Weapon.h"

/// Chaingun weapon
class Chaingun : public Weapon
{
public:
  Chaingun(Entity *pShooter);
  ~Chaingun();

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void);
  /// Animates the weapon
  virtual void Animate(void);
  /// Fires the weapon
  virtual void Fire(void);

public:
  float m_fBarrelRotation;
  float m_fBarrelRotationSpeed;
  int m_iBarrelSprite;
};