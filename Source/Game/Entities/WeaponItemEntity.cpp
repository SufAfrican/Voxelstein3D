#include "StdAfx.h"
#include "Game/Entities/WeaponItemEntity.h"
#include "Game/Entities/PlayerEntity.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
////////////////////////////////////////////
SXLPARSER_Start("kv6\\luger.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_Pistol(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\lugerclip.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_PistolClip(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\chaingun.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_Chaingun(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\ammobox.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_ChaingunAmmo(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\TNTBUNDL.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_Grenade(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\c4.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(CreateWeaponItem_StickyBomb(mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////

WeaponItemEntity *CreateWeaponItem_Pistol(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\luger.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_Pistol;
  pEntity->m_bGiveWeapon = true;
  pEntity->m_iGiveAmmo = Config::Pistol_AmmoInWeapon;
  return pEntity;
}

WeaponItemEntity *CreateWeaponItem_PistolClip(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\lugerclip.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_Pistol;
  pEntity->m_bGiveWeapon = false;
  pEntity->m_iGiveAmmo = Config::Pistol_AmmoInClip;
  return pEntity;
}

WeaponItemEntity *CreateWeaponItem_Chaingun(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\chaingun.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_Chaingun;
  pEntity->m_bGiveWeapon = true;
  pEntity->m_iGiveAmmo = Config::Chaingun_AmmoInWeapon;
  pEntity->m_strPickupSound = "ogg/chaingunpickup.ogg";
  pEntity->m_fPickupSoundVolume = 0.5f;
  return pEntity;
}

WeaponItemEntity *CreateWeaponItem_ChaingunAmmo(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\ammobox.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_Chaingun;
  pEntity->m_bGiveWeapon = false;
  pEntity->m_iGiveAmmo = Config::Chaingun_AmmoInClip;
  return pEntity;
}

WeaponItemEntity *CreateWeaponItem_Grenade(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\TNTBUNDL.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_Grenade;
  pEntity->m_bGiveWeapon = true;
  pEntity->m_iGiveAmmo = 1;
  return pEntity;
}

WeaponItemEntity *CreateWeaponItem_StickyBomb(const Matrix44 &mTrans, bool bCreateSprite)
{
  WeaponItemEntity *pEntity = new WeaponItemEntity(bCreateSprite ? "kv6\\c4.kv6" : NULL, mTrans);
  pEntity->m_iWeaponIndex = PlayerEntity::Weapon_StickyBomb;
  pEntity->m_bGiveWeapon = true;
  pEntity->m_iGiveAmmo = 1;
  return pEntity;
}
////////////////////////////////////////////

WeaponItemEntity::WeaponItemEntity(const char *strSpriteFile, const Matrix44 & mTransform) : ItemEntity(strSpriteFile, mTransform)
{
  m_iWeaponIndex = 0;
  m_bGiveWeapon = false;
  m_iGiveAmmo = 0;
  m_strPickupSound = "ogg/ammopickup.ogg";

  m_fPickupSoundVolume = 0.3f;

  GetGame()->GetStats().iItems++;
}

WeaponItemEntity::~WeaponItemEntity()
{
}

void WeaponItemEntity::OnPickup(PlayerEntity *pPlayer)
{
  if(m_iWeaponIndex == PlayerEntity::Weapon_Chaingun)
    pPlayer->m_iLastFoundChaingun = GetGame()->GetCurrentTick();

  pPlayer->GiveWeaponItem((PlayerEntity::WeaponNumber)m_iWeaponIndex, m_bGiveWeapon, m_iGiveAmmo);
  pPlayer->GetStats().iItems++;

  if(m_strPickupSound != NULL)
  {
    Sound *pSound = GetSoundManager()->CreateSound2D(m_strPickupSound);
    pSound->SetVolume(m_fPickupSoundVolume);
    pSound->Play();
  }

  GetGame()->RemoveEntity(this);
}
