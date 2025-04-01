#include "StdAfx.h"
#include "Game/Entities/OfficerEntity.h"
#include "Game/Entities/WeaponItemEntity.h"
#include "Game/Weapons/Pistol.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
SXLPARSER_Start("kv6\\place_officer.kv6")
{
  mTransform.RemoveScale();
  bKeepSprite = false;

  OfficerEntity *pEntity = new OfficerEntity();
  pEntity->m_vPosition = mTransform.GetTranslation();
  pEntity->m_vRotation = GetEuler(mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();

OfficerEntity::OfficerEntity()
{
  m_Classes.push_back(Class_Officer);

  m_pWeapon = new Pistol(this);
  m_pWeapon->SetAmmo(INT_MAX);
  m_iShootDelay = Config::Officer_fShootDelay * Game::iTicksPerSecond;
  m_iMoveToNewPositionDelay = Config::Officer_fMoveToNewPositionDelay * Game::iTicksPerSecond;
  m_iShootPrepareDelay = Config::Officer_fShootPrepareDelay * Game::iTicksPerSecond;
  m_fShootInaccuracy = Config::Officer_fShootInaccuracy;
  m_iDamagedDelay = Config::Officer_fDamagedDelay * Game::iTicksPerSecond;

  m_pSprite = AnimationFileManager::GetInstance()->CreateFromFile("kv6/spion/spion.anim");
  m_pSprite->ListSprites(m_CollisionSprites);
  m_mSpriteRotation = MatrixFromEuler(Vector3D(Math::fPi,0,0));
  m_vSpriteScale = Vector3D(0.55f, 0.55f, 0.55f);
  m_vSpriteOffset = Vector3D(0,-35,0);

  m_iAnimGuard = m_pSprite->m_pFile->GetAnimation("idle");
  m_iAnimPrepareShoot = m_pSprite->m_pFile->GetAnimation("aim");
  m_iAnimShoot = m_pSprite->m_pFile->GetAnimation("shoot");
  m_iAnimWalk = m_pSprite->m_pFile->GetAnimation("run");
  m_iAnimHurt = m_pSprite->m_pFile->GetAnimation("hurt");
  m_iAnimDie = m_pSprite->m_pFile->GetAnimation("die");

  m_DamageSounds.push_back("Wolf3D:683076,2111,0,2111");
  m_DeathSounds.push_back("Wolf3D:710668,5859,1387528,8249");
  m_AlertSounds.push_back("Wolf3D:642075,4438,1381911,5030");

  m_fViewDistance = 1000.0f;
  m_fViewCosAngle = -0.1f;

  m_fCharacterHeight = 70.0f;
  m_vCollisionBox = Vector3D(30,70,30);
  m_vSize = Vector3D(50, 70, 50);
  m_bHasRagdoll = true;

  GotoState(State_Guard);
}

void OfficerEntity::OnCreateRagdoll(void)
{
  // spawn ammo
  SpritePoser *pSprite = m_pSprite->FindSprite("kv6\\spion\\spiongun.kv6");
  if(pSprite != NULL)
  {
    WeaponItemEntity *pItem = CreateWeaponItem_Pistol(pSprite->GetSprite()->GetTransform(), false);
    pItem->m_pSprite = pSprite->GetSprite();
    GetGame()->AddEntity(pItem);
  }
}