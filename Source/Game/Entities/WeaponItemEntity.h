#pragma once

#include "Game/Entities/ItemEntity.h"

/// Weapon or ammo pickup
class WeaponItemEntity : public ItemEntity
{
public:
  WeaponItemEntity(const char *strSpriteFile, const Matrix44 &mTransform);
  ~WeaponItemEntity();

  /// Called when player picks up the entity
  virtual void OnPickup(PlayerEntity *pPlayer);

public:
  /// Index of weapon
  int m_iWeaponIndex;
  /// Should give weapon or just ammo?
  bool m_bGiveWeapon;
  /// How much ammo to give
  int m_iGiveAmmo;
  /// Sound played on pickup
  const char *m_strPickupSound;
  /// Pickup sound volume
  float m_fPickupSoundVolume;
};

extern WeaponItemEntity *CreateWeaponItem_Pistol(const Matrix44 &mTrans, bool bCreateSprite = true);
extern WeaponItemEntity *CreateWeaponItem_PistolClip(const Matrix44 &mTrans, bool bCreateSprite = true);
extern WeaponItemEntity *CreateWeaponItem_Chaingun(const Matrix44 &mTrans, bool bCreateSprite = true);
extern WeaponItemEntity *CreateWeaponItem_ChaingunAmmo(const Matrix44 &mTrans, bool bCreateSprite = true);
extern WeaponItemEntity *CreateWeaponItem_Grenade(const Matrix44 &mTrans, bool bCreateSprite = true);
extern WeaponItemEntity *CreateWeaponItem_StickyBomb(const Matrix44 &mTrans, bool bCreateSprite = true);