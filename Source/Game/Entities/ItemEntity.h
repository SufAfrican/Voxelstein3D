#pragma once

#include "Game/Entities/Entity.h"

class PlayerEntity;

/// Pickup-able item
class ItemEntity : public Entity
{
public:
  ItemEntity();
  ItemEntity(const char *strSpriteFile, const Matrix44 &mTransform);
  ~ItemEntity();

  /// Called when player picks up the entity
  virtual void OnPickup(PlayerEntity *pPlayer) = 0;

  /// Updates item position when custom sprite is given
  virtual void Tick(void);

public:
  /// Pickup sprite
  Sprite *m_pSprite;
  /// Is sprite owned by this entity
  bool m_bOwnedSprite;
};