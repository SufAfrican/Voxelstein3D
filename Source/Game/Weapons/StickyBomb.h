#pragma once

#include "Game/Weapons/Weapon.h"

/// Sticky explosive weapon
class StickyBomb : public Weapon
{
public:
  StickyBomb(Entity *pShooter);
  ~StickyBomb();

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void);
  /// Fires the weapon
  virtual void Fire(void);
  /// Animates the weapon
  virtual void Animate(void);
};