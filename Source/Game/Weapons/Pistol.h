#pragma once

#include "Game/Weapons/Weapon.h"

/// Pistol weapon
class Pistol : public Weapon
{
public:
  Pistol(Entity *pShooter);
  ~Pistol();

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void);
  /// Fires the weapon
  virtual void Fire(void);  
};
