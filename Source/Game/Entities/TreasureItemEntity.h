#pragma once

#include "Game/Entities/ItemEntity.h"

/// Treasure pickup
class TreasureItemEntity : public ItemEntity
{
public:
  TreasureItemEntity(const char *strSpriteFile, const Matrix44 &mTransform);

  /// Called when player picks up the entity
  virtual void OnPickup(PlayerEntity *pPlayer);

public:
  /// Sound played on pickup
  const char *m_strPickupSound;
  /// Treasure gives player flashlight
  bool m_bGiveFlashlight;
};
