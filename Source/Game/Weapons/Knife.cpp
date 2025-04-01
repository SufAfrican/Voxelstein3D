#include "StdAfx.h"
#include "Game/Weapons/Knife.h"
#include "Game/Entities/Entity.h"
#include "Game/ParticleSystems/ParticleSystem.h"

Knife::Knife(Entity *pShooter) : Weapon(pShooter)
{
  m_iFireDelay = Config::Knife_fStabDelay * Game::fTicksPerSecond;
  m_bFiresByClick = Config::Knife_bFiresByClick;

  m_fSwingingSpeed = Config::Knife_fSwingingSpeed;
  m_fSwingingAmount = Config::Knife_fSwingingAmount;

  m_iAmmo = -1;
}

Knife::~Knife()
{
}

void Knife::CreateFirstPersonSprites(void)
{
  m_pHUDImage = Voxlap::LoadImage("png/hud_knife.png");

  m_pWeaponSprite = AnimationFileManager::GetInstance()->CreateFromFile("kv6\\firstperson\\knife.anim");
  if(m_pWeaponSprite)
  {
    m_pWeaponSprite->m_iCurrentAnim = 0;
    m_pWeaponSprite->m_bPlaying = true;
    m_pWeaponSprite->m_bLooping = true;

    m_iAnimIdle = m_pWeaponSprite->m_pFile->GetAnimation("Idle");
    m_iAnimShoot = m_pWeaponSprite->m_pFile->GetAnimation("Stab");

    for(unsigned int i = 0; i < m_pWeaponSprite->m_Sprites.size(); i++)
    {
      m_pWeaponSprite->m_Sprites[i]->GetSprite()->SetRenderInFront(true);
    }
  }
}

void Knife::Fire(void)
{
  // shooting sound
  Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/knife.ogg");
  pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
  m_FiringSounds.push_back(pSound);

  Entity::HitInfo info;

  if(m_pShooter->HitScan(m_vLaunchPos, m_vLaunchDir, Config::Knife_fHitDistance, info))
  {
    if(info.pEntity)
    {
      Entity::DamageInfo dmg;
      dmg.Type = Entity::DamageType_Blade;
      dmg.iAmount = GetRandomInt(Config::Knife_iDamageMin, Config::Knife_iDamageMax);
      dmg.pHitSprite = info.pSprite;
      dmg.vHitDir = m_vLaunchDir;
      dmg.vHitNormal = info.vNormal;
      dmg.vHitPos = info.vPos;
      info.pEntity->ReceiveDamage(dmg, m_pShooter);
    }
    if(!info.pEntity)
    {
      Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/knife_hit.ogg");
      pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
      m_FiringSounds.push_back(pSound);
    }

    Voxlap::DrawVoxelBulletHole(info.vPos + Vector3D(0,0,-1));
    Voxlap::DrawVoxelBulletHole(info.vPos + Vector3D(0,0,1));
    Voxlap::DrawVoxelBulletHole(info.vPos + Vector3D(1,0,0));
    Voxlap::DrawVoxelBulletHole(info.vPos + Vector3D(-1,0,0));
    Voxlap::DrawVoxelBulletHole(info.vPos);
  }

  // play shooting anim
  if(m_pWeaponSprite)
  {
    m_pWeaponSprite->m_fCurrentTime = 0.0f;
    m_pWeaponSprite->m_iCurrentAnim = m_iAnimShoot;
    m_pWeaponSprite->m_bLooping = false;
  }
}
