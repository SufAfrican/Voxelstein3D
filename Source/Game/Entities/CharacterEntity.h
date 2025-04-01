#pragma once

#include "Game/Entities/Entity.h"

/// Base class for player and enemies
class CharacterEntity : public Entity
{
public:
  CharacterEntity();
  ~CharacterEntity();

  /// Updates the entity state
  virtual void Tick(void);

  /// Attemps to move the character by given offset, returns the offset that is possible to move
  Vector3D CharacterClipMove(const Vector3D &vMove);

public:
  /// Height relative to position where character sees from
  float m_fEyeHeight;
  /// Radius of character (fatness)
  float m_fCharacterRadius;
  /// Height of character
  float m_fCharacterHeight;
  /// How high character can climb (stairs etc.)
  float m_fCharacterStepHeight;

  /// Is character currently on floor
  bool m_bIsOnFloor;
  /// Is character movement collision enabled
  bool m_bCharacterMoveClip;
};