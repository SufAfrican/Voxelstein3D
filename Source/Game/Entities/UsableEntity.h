#pragma once

#include "Game/Entities/Entity.h"

/// Base class for things that can be used by the player
class UsableEntity : public Entity
{
public:
  UsableEntity();
  
  /// Player pressed use
  virtual void PressedUse(Entity *pEntity, const Vector3D &vSource, const Vector3D &vHit) = 0;
};