#pragma once

#include "Game/Entities/Entity.h"

/// Simple entity that handles collision for a map sprite
class DummySpriteEntity : public Entity
{
public:
  DummySpriteEntity(const char *strFile, const Matrix44 &mTransform);
  ~DummySpriteEntity();

  /// Entity was damaged
  virtual void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager);

public:
  /// Sprite
  Sprite *m_pSprite;
};