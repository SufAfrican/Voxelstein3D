#pragma once

#include "Game/Weapons/Weapon.h"

/// Knife weapon
class Knife : public Weapon
{
public:
  Knife(Entity *pShooter);
  ~Knife();

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void);
  /// Fires the weapon
  virtual void Fire(void);  
};