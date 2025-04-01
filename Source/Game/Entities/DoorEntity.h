#pragma once

#include "Game/Entities/Entity.h"

struct vx5sprite;

/// Openable door entity
class DoorEntity : public Entity
{
public:
  DoorEntity(const char *strSpriteFile, const Matrix44 &mTrans);
  ~DoorEntity();

  /// Entity was damaged
  void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager);

  /// Opens/closes the door away from the opener
  void ToggleDoorOpen(const Vector3D &vOpener);
  /// Updates collision box depending where door is facing
  void UpdateCollision(void);

  /// Updates the door state
  void Tick(void);

public:
  Sprite *m_pSprite;

  /// Original transform of the door
  Matrix44 m_mOriginal;
  /// Current rotation
  float m_fCurrentRotation;
  /// Target rotation
  float m_fTargetRotation;
  /// Is opening or closing
  bool m_bMoving;

  /// Is this a sliding door
  bool m_bSlide;
};