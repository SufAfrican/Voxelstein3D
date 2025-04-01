#include "StdAfx.h"
#include "Game/Weapons/Pistol.h"
#include "Game/Entities/Entity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/BulletHit.h"
#include "Game/ParticleSystems/Muzzle.h"

Pistol::Pistol(Entity *pShooter) : Weapon(pShooter)
{
  m_iFireDelay = Config::Pistol_fFireDelay * Game::fTicksPerSecond;
  m_bFiresByClick = Config::Pistol_bFiresByClick;

  m_fSwingingSpeed = Config::Pistol_fSwingingSpeed;
  m_fSwingingAmount = Config::Pistol_fSwingingAmount;

  m_iAmmo = 0;
}

Pistol::~Pistol()
{
}

void Pistol::CreateFirstPersonSprites(void)
{
  m_pHUDImage = Voxlap::LoadImage("png/hud_luger.png");

  m_pWeaponSprite = AnimationFileManager::GetInstance()->CreateFromFile("kv6\\firstperson\\luger.anim");
  if(m_pWeaponSprite)
  {
    m_pWeaponSprite->m_iCurrentAnim = 0;
    m_pWeaponSprite->m_bPlaying = true;
    m_pWeaponSprite->m_bLooping = true;

    m_iAnimIdle = m_pWeaponSprite->m_pFile->GetAnimation("Idle");
    m_iAnimShoot = m_pWeaponSprite->m_pFile->GetAnimation("Shoot");

    for(unsigned int i = 0; i < m_pWeaponSprite->m_Sprites.size(); i++)
    {
      m_pWeaponSprite->m_Sprites[i]->GetSprite()->SetRenderInFront(true);

      if(m_pWeaponSprite->m_Sprites[i]->GetSprite()->GetFilename().find("muzzle_effect_pointer.kv6") != std::string::npos)
      {
        m_pWeaponSprite->m_Sprites[i]->GetSprite()->SetVisible(false);
        m_iMuzzleSpriteIndex = i;
      }
    }
  }
}

void Pistol::Fire(void)
{
  if(m_iAmmo <= 0)
  {
    // empty sound
    Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/empty.ogg");
    pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
    m_FiringSounds.push_back(pSound);

    GetSoundManager()->CreateSound("ogg/weapons/empty.ogg")->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
    return;
  }

  // shooting sound (2D for first person)
  const char *strShootingSound = "Wolf3D:633857,3593,0,3593";
  Sound *pSound = m_pWeaponSprite ? GetSoundManager()->CreateSound2D(strShootingSound) : GetSoundManager()->CreateSound(strShootingSound);
  pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
  m_FiringSounds.push_back(pSound);

  // notify enemies
  m_pShooter->SendSoundEvent(m_vLaunchPos, 400.0f);

  Entity::HitInfo info;

  if(m_pShooter->HitScan(m_vLaunchPos, m_vLaunchDir, Config::Pistol_fFireDistance, info))
  {
    if(info.pEntity)
    {
      Entity::DamageInfo dmg;
      dmg.iAmount = GetRandomInt(Config::Pistol_iDamageMin, Config::Pistol_iDamageMax);
      dmg.pHitSprite = info.pSprite;
      dmg.vHitDir = m_vLaunchDir;
      dmg.vHitNormal = info.vNormal;
      dmg.vHitPos = info.vPos;
      info.pEntity->ReceiveDamage(dmg, m_pShooter);
    }
    else
    {
      Voxlap::DrawVoxelBulletHole(info.vPos);
      GetParticles()->CreateSystem<BulletHitParticleSystem>()->Start(info.vPos, info.vNormal, info.iColor);
      PlayRicochetSound(info.vPos);
    }
  }

  // muzzle flash
  if(m_pWeaponSprite && m_iMuzzleSpriteIndex >= 0)
  {
    const Matrix44 &mGun = m_pWeaponSprite->m_Sprites[m_iMuzzleSpriteIndex]->GetSprite()->GetTransform();
    Vector3D vMuzzlePos = mGun.GetTranslation();
    Vector3D vMuzzleDir = mGun.GetFrontVector();
    GetParticles()->CreateSystem<MuzzleParticleSystem>()->Start(vMuzzlePos, vMuzzleDir);
  }

  // play shooting anim
  if(m_pWeaponSprite)
  {
    m_pWeaponSprite->m_fCurrentTime = 0.0f;
    m_pWeaponSprite->m_iCurrentAnim = m_iAnimShoot;
    m_pWeaponSprite->m_bLooping = false;
  }

  m_iAmmo--;
}
