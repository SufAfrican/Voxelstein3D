#include "StdAfx.h"
#include "Game/Weapons/Chaingun.h"
#include "Game/Entities/Entity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/BulletHit.h"
#include "Game/ParticleSystems/Muzzle.h"
#include "Engine/Core/VoxelRayTraverse.h"

Chaingun::Chaingun(Entity *pShooter) : Weapon(pShooter)
{
  m_iFireDelay = Config::Chaingun_fFireDelay * Game::fTicksPerSecond;
  m_bFiresByClick = Config::Chaingun_bFiresByClick;

  m_fSwingingSpeed = Config::Chaingun_fSwingingSpeed;
  m_fSwingingAmount = Config::Chaingun_fSwingingAmount;

  m_fBarrelRotationSpeed = 0;
  m_fBarrelRotation = 0;

  m_iAmmo = 0;
}

Chaingun::~Chaingun()
{
}

void Chaingun::CreateFirstPersonSprites(void)
{
  m_pHUDImage = Voxlap::LoadImage("png/hud_chaingun.png");

  m_pWeaponSprite = AnimationFileManager::GetInstance()->CreateFromFile("kv6\\firstperson\\chaingun.anim");
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

      if(m_pWeaponSprite->m_Sprites[i]->GetSprite()->GetFilename().find("barrel") != std::string::npos)
      {
        m_iBarrelSprite = i;
      }
    }
  }
}

void Chaingun::Animate(void)
{
  Weapon::Animate();

  m_fBarrelRotation += m_fBarrelRotationSpeed;
  m_fBarrelRotationSpeed *= 0.95f;
  Quaternion qBarrelRotation = QuaternionFromEuler(Vector3D(0,0,m_fBarrelRotation));
  Quaternion &qRot = m_pWeaponSprite->m_Sprites[m_iBarrelSprite]->m_qRotation;
  qRot = qBarrelRotation * qRot;
  m_pWeaponSprite->m_Sprites[m_iBarrelSprite]->UpdateSprite(m_pWeaponSprite->GetTransform());
}

void Chaingun::Fire(void)
{
  if(m_iAmmo <= 0)
  {
    // empty sound
    Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/empty.ogg");
    pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
    m_FiringSounds.push_back(pSound);

    // extra delay
    m_iLastFire += Game::iTicksPerSecond / 4;
    return;
  }

  // shooting sound (2D for first person)
  const char *strShootingSound = "Wolf3D:637978,4033,0,4033";
  Sound *pSound = m_pWeaponSprite ? GetSoundManager()->CreateSound2D(strShootingSound) : GetSoundManager()->CreateSound(strShootingSound);
  pSound->Play(m_vLaunchPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);
  m_FiringSounds.push_back(pSound);

  // notify enemies
  m_pShooter->SendSoundEvent(m_vLaunchPos, 400.0f);

  Entity::HitInfo info;

  Vector3D vRandDir(-0.5f + rand()/(float)RAND_MAX,
                    -0.5f + rand()/(float)RAND_MAX,
                    -0.5f + rand()/(float)RAND_MAX);
  Vector3D vLaunchDir = Normalize(m_vLaunchDir + vRandDir * Config::Chaingun_fFireSpreadFactor);
  Vector3D vLaunchPos = m_vLaunchPos;

  // do two traces
  for(int i = 0; i < 2; i++)
  {
    if(m_pShooter->HitScan(vLaunchPos, vLaunchDir, Config::Chaingun_fFireDistance, info))
    {
      if(i == 0)
      {
        if(info.pEntity)
        {
          Entity::DamageInfo dmg;
          dmg.iAmount = GetRandomInt(Config::Chaingun_iDamageMin, Config::Chaingun_iDamageMax);
          dmg.pHitSprite = info.pSprite;
          dmg.vHitDir = m_vLaunchDir;
          dmg.vHitNormal = info.vNormal;
          dmg.vHitPos = info.vPos;
          info.pEntity->ReceiveDamage(dmg, m_pShooter);
        }
        else
        {
          Voxlap::DrawVoxelBulletHoleBig(info.vPos);
        }

        // cut a penetration depth
        VoxelRayTraverse trav(info.vPos, vLaunchDir);
        for(int i = 0; i < Config::Chaingun_fBulletPenetrationDepth; i++)
        {
          const Vector3D &vStep = trav.Step();

          if(Config::Debug_bVisualizeBulletPenetration)
            Voxlap::SetVoxelColor(vStep,255,0,0);
          else
            Voxlap::DrawVoxelBulletHole(vStep);
        }

        // shoot 2nd ray through the hole
        vLaunchPos = info.vPos; 
      }
      else
      {
        if(info.pEntity)
        {
          Entity::DamageInfo dmg;
          dmg.iAmount = GetRandomInt(Config::Chaingun_iDamageMinPenetrated, Config::Chaingun_iDamageMaxPenetrated);
          dmg.pHitSprite = info.pSprite;
          dmg.vHitDir = m_vLaunchDir;
          dmg.vHitNormal = info.vNormal;
          dmg.vHitPos = info.vPos;
          info.pEntity->ReceiveDamage(dmg, m_pShooter);
        }
        else
        {
          // 2nd ray just makes a tiny hole
          Voxlap::DrawVoxelBulletHole(info.vPos);
        }
      }
  
      if(!info.pEntity)
      {
        GetParticles()->CreateSystem<BulletHitParticleSystem>()->Start(info.vPos, info.vNormal, info.iColor);
        PlayRicochetSound(info.vPos);
      }
    }
  }

  // muzzle flash
  if(m_pWeaponSprite && m_iMuzzleSpriteIndex >= 0)
  {
    const Matrix44 &mGun = m_pWeaponSprite->m_Sprites[m_iMuzzleSpriteIndex]->GetSprite()->GetTransform();
    Vector3D vMuzzlePos = mGun.GetTranslation();
    Vector3D vMuzzleDir = mGun.GetFrontVector();
    GetParticles()->CreateSystem<MuzzleParticleSystem>()->Start(vMuzzlePos, vMuzzleDir, 1, 2.0f, 2.0f, "png/muzzlebig.png");
  }

  // play shooting anim
  if(m_pWeaponSprite)
  {
    m_pWeaponSprite->m_fCurrentTime = 0.0f;
    m_pWeaponSprite->m_iCurrentAnim = m_iAnimShoot;
    m_pWeaponSprite->m_bLooping = false;
  }

  m_fBarrelRotationSpeed = 0.1f;
  m_iAmmo--;
}
