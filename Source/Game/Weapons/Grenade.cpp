#include "StdAfx.h"
#include "Game/Weapons/Grenade.h"
#include "Game/Entities/Entity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/Explosion.h"

Grenade::Grenade(Entity *pShooter) : Weapon(pShooter)
{
  m_iFireDelay = Config::Grenade_fFireDelay * Game::fTicksPerSecond;
  m_bFiresByClick = Config::Grenade_bFiresByClick;
  m_iAmmo = 0;
}

Grenade::~Grenade()
{
  for(unsigned int i = 0; i < m_Grenades.size(); i++)
  {
    GetPhysics()->RemoveStick(m_Grenades[i]->Stick);
    delete m_Grenades[i]->pSprite;
    delete m_Grenades[i];
  }
}

void Grenade::CreateFirstPersonSprites(void)
{
  m_pHUDImage = Voxlap::LoadImage("png/hud_gren.png");

  // todo..
}

void Grenade::Animate(void)
{
  Weapon::Animate();

  float fDeltaTime = GetGame()->GetTickDuration();

  Matrix44 mCorrection = MatrixFromEuler(Vector3D(0,0,-0.2f));
  mCorrection.Scale(Vector3D(0.15f, 0.15f, 0.15f));
  mCorrection.SetTranslation(Vector3D(-5,0,0));

  for(unsigned int i = 0; i < m_Grenades.size();)
  {
    Projectile *pProj = m_Grenades[i];
    pProj->fTime += fDeltaTime;

    Matrix44 mStick;
    GetPhysics()->GetTransform(pProj->Stick, mStick);
    pProj->pSprite->SetTransform(Multiply(mCorrection, mStick));

    Vector3D vPos = mStick.GetTranslation();

    if(pProj->fTime > Config::Grenade_fExplodeTime)
    {
      // just shoot a lot of death rays
      std::vector<Entity *> Entities;
      Vector3D vRadius(Config::Grenade_fDamageRadius,Config::Grenade_fDamageRadius,Config::Grenade_fDamageRadius);
      GetGame()->GetEntities(vPos - vRadius, vPos + vRadius, Entities);
      for(unsigned int j = 0; j < Entities.size(); j++)
      {
        Entity *pEntity = Entities[j];
        Vector3D vDir = pEntity->m_vPosition - vPos;
        vDir.Normalize();

        for(int r = 0; r < Config::Grenade_iDamageRays; r++)
        {
          Entity::HitInfo info;
          vDir = Normalize(vDir + 0.25f * Vector3D(-0.5f + rand()/(float)RAND_MAX,
                                   -0.5f + rand()/(float)RAND_MAX,
                                   -0.5f + rand()/(float)RAND_MAX));
          Entity *pDummy = NULL;
          if(pDummy->HitScan(vPos, vDir, Config::Grenade_fDamageRadius, info) && info.pEntity == pEntity)
          {
            float fHitDistanceFactor = (1.0f - Clamp(info.fDistance / Config::Grenade_fDamageRadius, 0.0f, 0.9f));
            Entity::DamageInfo dmg;
            dmg.Type = Entity::DamageType_Explosion;
            dmg.iAmount = (GetRandomInt(Config::Grenade_iDamageMin, Config::Grenade_iDamageMax)) * fHitDistanceFactor;
            dmg.vHitPos = info.vPos;
            dmg.vHitNormal = info.vNormal;
            dmg.vHitDir = vDir;
            dmg.pHitSprite = info.pSprite;
            pEntity->ReceiveDamage(dmg, m_pShooter);
          }
        }
      }

      // explosion sound
      Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/grenade.ogg");
      pSound->SetRange(300.0f, 2000.0f);
      pSound->Play(vPos, 0.9f + 0.1f * rand()/(float)RAND_MAX);

      // notify enemies
      m_pShooter->SendSoundEvent(vPos, 800.0f);

      float fAngle = 2.0f * Math::fPi * rand()/(float)RAND_MAX;
      for(int x = 0; x < 12; x++)
      {
        fAngle += Math::fPi * 0.1f + 0.5f * rand()/(float)RAND_MAX;
        float fTrackSize = Config::Grenade_fExplodeRadius * (0.3f + 0.1f * rand()/(float)RAND_MAX);
        Vector3D vTrackPos = vPos;
        Vector3D vTrackDir(0,0,0);
        if(x < 4)
        {
          vTrackDir[0] = sinf(fAngle);
          vTrackDir[2] = -cosf(fAngle);
        }
        else if(x < 8)
        {
          vTrackDir[0] = sinf(fAngle);
          vTrackDir[1] = -cosf(fAngle);
        }
        else
        {
          vTrackDir[1] = sinf(fAngle);
          vTrackDir[2] = -cosf(fAngle);
        }
        while(fTrackSize > 1.0f)
        {
          vTrackPos += vTrackDir * fTrackSize;
          fTrackSize *= 0.8f;
          Voxlap::ClearVoxelSphere(vTrackPos, fTrackSize);
        }
      }
      Voxlap::DrawVoxelExplosionHole(vPos, Config::Grenade_fExplodeRadius);

      GetParticles()->CreateSystem<ExplosionParticleSystem>()->Start(vPos, Config::Grenade_fExplodeRadius);
      GetPhysics()->RemoveStick(pProj->Stick);
      delete pProj->pSprite;
      delete pProj;
      m_Grenades[i] = m_Grenades.back();
      m_Grenades.pop_back();
      continue;
    }
    else
    {
       i++;
    }
  }
}

void Grenade::Fire(void)
{
  if(m_iAmmo <= 0)
    return;

  Projectile *pProj = new Projectile();
  pProj->fTime = 0.0f;
  pProj->pSprite = new Sprite("kv6/kgrenade.kv6");

  GetPhysics()->AddStick(pProj->Stick, m_vLaunchPos, Vector3D(1,0,0), 12.0f, 2.0f);
  Vector3D vVel = m_vLaunchDir * (Config::Grenade_fThrowSpeed * GetGame()->GetTickDuration());
  GetPhysics()->SetVelocity(pProj->Stick, vVel*0.5f);
  GetPhysics()->GetParticle(pProj->Stick.iParticle[0])->vOldPos -= vVel*0.25f;
  GetPhysics()->GetParticle(pProj->Stick.iParticle[2])->vOldPos += vVel*0.25f;
  m_Grenades.push_back(pProj);

  m_iAmmo--;
}
