#include "StdAfx.h"
#include "Game/Entities/GuardEntity.h"
#include "Game/Entities/WeaponItemEntity.h"
#include "Game/Weapons/Pistol.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
SXLPARSER_Start("kv6\\place_guard.kv6")
{
  mTransform.RemoveScale();
  bKeepSprite = false;

  GuardEntity *pEntity = new GuardEntity();
  pEntity->m_vPosition = mTransform.GetTranslation();
  pEntity->m_vRotation = GetEuler(mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();

static const char *_strGores[2] = { "kv6\\guard2\\bbrain.kv6", "kv6\\guard2\\bbonehead.kv6" };

GuardEntity::GuardEntity()
{
  m_Classes.push_back(Class_Guard);

  m_pWeapon = new Pistol(this);
  m_pWeapon->SetAmmo(INT_MAX);
  m_iShootDelay = Config::Guard_fShootDelay * Game::iTicksPerSecond;
  m_iMoveToNewPositionDelay = Config::Guard_fMoveToNewPositionDelay * Game::iTicksPerSecond;
  m_iShootPrepareDelay = Config::Guard_fShootPrepareDelay * Game::iTicksPerSecond;
  m_fShootInaccuracy = Config::Guard_fShootInaccuracy;
  m_iDamagedDelay = Config::Guard_fDamagedDelay * Game::iTicksPerSecond;

  m_pSprite = AnimationFileManager::GetInstance()->CreateFromFile("kv6/guard2/guard2.anim");
  m_pSprite->ListSprites(m_CollisionSprites);
  m_vSpriteScale = Vector3D(0.35f, 0.35f, 0.35f);
  m_vSpriteOffset = Vector3D(0,-35,0);

  m_iAnimGuard = m_pSprite->m_pFile->GetAnimation("idle");
  m_iAnimPrepareShoot = m_pSprite->m_pFile->GetAnimation("aim");
  m_iAnimShoot = m_pSprite->m_pFile->GetAnimation("shoot");
  m_iAnimWalk = m_pSprite->m_pFile->GetAnimation("walk");
  m_iAnimHurt = m_pSprite->m_pFile->GetAnimation("hurt");
  m_iAnimDie = m_pSprite->m_pFile->GetAnimation("die");

  m_DamageSounds.push_back("Wolf3D:683076,2111,0,2111");
  m_DeathSounds.push_back("Wolf3D:675948,2647,0,2647");
  m_DeathSounds.push_back("Wolf3D:678947,3695,0,3695");
  m_AlertSounds.push_back("Wolf3D:590607,4150,0,4150");

  m_fViewDistance = 1000.0f;
  m_fViewCosAngle = -0.1f;

  m_iHealth = Config::Guard_iHealth;

  m_fCharacterHeight = 70.0f;
  m_vCollisionBox = Vector3D(30,70,30);
  m_vSize = Vector3D(50, 70, 50);
  m_bHasRagdoll = true;

  m_IndestructibleSprites.push_back( m_pSprite->FindSprite("kv6\\guard2\\spiongun.kv6") );
  m_IndestructibleSprites.push_back( m_pSprite->FindSprite("kv6\\guard2\\nade.kv6") );
  m_IndestructibleSprites.push_back( m_pSprite->FindSprite("kv6\\guard2\\helmet.kv6") );

  // hide gore
  for(int i = 0; i < 2; i++)
  {
    SpritePoser *pSprite = m_pSprite->FindSprite(_strGores[i]);
    if(pSprite != NULL)
      pSprite->GetSprite()->SetVisible(false);
  }

  GotoState(State_Guard);
}

void GuardEntity::OnCreateRagdoll(void)
{
  // show gore
  for(int i = 0; i < 2; i++)
  {
    SpritePoser *pSprite = m_pSprite->FindSprite(_strGores[i]);
    if(pSprite != NULL)
      pSprite->GetSprite()->SetVisible(true);
  }

  // spawn ammo
  SpritePoser *pSprite = m_pSprite->FindSprite("kv6\\guard2\\spiongun.kv6");
  if(pSprite != NULL)
  {
    WeaponItemEntity *pItem = CreateWeaponItem_Pistol(pSprite->GetSprite()->GetTransform(), false);
    pItem->m_pSprite = pSprite->GetSprite();
    GetGame()->AddEntity(pItem);
  }

  pSprite = m_pSprite->FindSprite("kv6\\guard2\\nade.kv6");
  if(pSprite != NULL)
  {
    WeaponItemEntity *pItem = CreateWeaponItem_Grenade(pSprite->GetSprite()->GetTransform(), false);
    pItem->m_pSprite = pSprite->GetSprite();
    GetGame()->AddEntity(pItem);
  }
}