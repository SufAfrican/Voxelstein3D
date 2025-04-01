#include "StdAfx.h"
#include "Game/Entities/StickyBombEntity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/ExplosionCloud.h"
#include "Game/ParticleSystems/ExplosionSmoke.h"

#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"


StickyBombEntity::StickyBombEntity(Entity *pShooter, const Vector3D &vPos, const Vector3D &vNormal)
{
  m_pShooter = pShooter;
  m_pSprite = new Sprite("kv6/c4.kv6");

  Vector3D vUp(0,0,0);
  if(fabsf(vNormal.y) > 0.9f)
    vUp.z = 1;
  else
    vUp.y = 1;

  Vector3D vSide = vNormal;
  Vector3D vFront = Normalize(Cross(vUp, vSide));
  vUp = -Normalize(Cross(vFront, vSide));

  Matrix44 mTrans(vSide.x,  vSide.y,  vSide.z, 0,
                    vUp.x,    vUp.y,    vUp.z, 0,
                 vFront.x, vFront.y, vFront.z, 0,
                        0,        0,        0, 1);
  mTrans.Scale(Vector3D(0.25f, 0.25f, 0.25f));
  mTrans.SetTranslation(vPos);
  m_pSprite->SetTransform(mTrans);

  m_vPosition = mTrans.GetTranslation();
  m_vNormal = vNormal;

  m_iTime = 0;
  m_bDetonated = false;
}

StickyBombEntity::~StickyBombEntity()
{
  delete m_pSprite;
}

void StickyBombEntity::Detonate(void)
{
  m_bDetonated = true;

  delete m_pSprite;
  m_pSprite = NULL;

  // explosion sound
  Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/c4.ogg");
  pSound->SetRange(300.0f, 2000.0f);
  pSound->Play(m_vPosition, 0.9f + 0.1f * rand()/(float)RAND_MAX);

  // notify enemies
  SendSoundEvent(m_vPosition, 800.0f);

  // just shoot a lot of death rays
  {
    std::vector<Entity *> Entities;
    float fDamageRadius = Config::StickyBomb_fDamageRayRadius;
    Vector3D vPos = m_vPosition + m_vNormal;
    Vector3D vRadius(fDamageRadius,fDamageRadius,fDamageRadius);
    GetGame()->GetEntities(vPos - vRadius, vPos + vRadius, Entities);
    for(unsigned int j = 0; j < Entities.size(); j++)
    {
      Entity *pEntity = Entities[j];
      Vector3D vDir = pEntity->m_vPosition - vPos;
      vDir.Normalize();

      for(int r = 0; r < Config::StickyBomb_iDamageRayAmount; r++)
      {
        Entity::HitInfo info;
        vDir = Normalize(vDir + 0.25f * Vector3D(-0.5f + rand()/(float)RAND_MAX,
                                 -0.5f + rand()/(float)RAND_MAX,
                                 -0.5f + rand()/(float)RAND_MAX));
        Entity *pDummy = NULL;
        if(pDummy->HitScan(vPos, vDir, fDamageRadius, info) && info.pEntity == pEntity)
        {
          float fHitDistanceFactor = (1.0f - Clamp(info.fDistance / fDamageRadius, 0.0f, 0.9f));
          Entity::DamageInfo dmg;
          dmg.Type = Entity::DamageType_Explosion;
          dmg.iAmount = (GetRandomInt(Config::StickyBomb_iDamageRayMin, Config::StickyBomb_iDamageRayMax)) * fHitDistanceFactor;
          dmg.vHitPos = info.vPos;
          dmg.vHitNormal = info.vNormal;
          dmg.vHitDir = vDir;
          dmg.pHitSprite = info.pSprite;
          pEntity->ReceiveDamage(dmg, this);
        }
      }
    }
  }

  // create particles
  int iParticlesX = Config::StickyBomb_iCloudsX;
  int iParticlesY = Config::StickyBomb_iCloudsY;
  m_Explosions.resize(iParticlesX*iParticlesY);
  int c = 0;
  for(int x = 0; x < iParticlesX; x++)
  for(int y = 0; y < iParticlesY; y++)
  {
    float fAngle1 = x/(float)iParticlesX*2.0f*Math::fPi + (-0.5f + rand()/(float)RAND_MAX)*0.1f;
    float fAngle2 = y/(float)iParticlesY*Math::fPi + (-0.5f + rand()/(float)RAND_MAX)*0.1f;
    m_Explosions[c].vPos = m_vPosition + m_vNormal;
    m_Explosions[c].vDir = GetDirectionVector(Vector3D(fAngle1, fAngle2, 0));
    m_Explosions[c].fRadius = GetRandomFloat(Config::StickyBomb_fCloudRadiusMin, Config::StickyBomb_fCloudRadiusMax);
    m_Explosions[c].fEnergy = 0.9f + (-0.5f + rand()/(float)RAND_MAX)*0.2f;
    m_Explosions[c].pParticle = GetParticles()->CreateSystem<ExplosionCloudParticleSystem>();
    c++;
  }

  TickExplosions();
}

void StickyBombEntity::TickExplosions(void)
{
  // update explosion clouds
  for(unsigned int i = 0; i < m_Explosions.size();)
  {
    ExplosionNode &node = m_Explosions[i];
    if(node.fEnergy <= 0.1f)
    {
      GetParticles()->CreateSystem<ExplosionSmokeParticleSystem>()->Start(node.vPos,
        node.vDir * rand()/(float)RAND_MAX * 50.0f,
        node.fRadius,
        0.1f + 0.1f * rand()/(float)RAND_MAX);
      node.pParticle->Finish();
      m_Explosions[i] = m_Explosions.back();
      m_Explosions.pop_back();
      continue;
    }

    node.fEnergy *= Config::StickyBomb_fCloudEnergyDecay;

    node.vDir = Normalize(node.vDir + Vector3D(0,Config::StickyBomb_fCloudLift,0));

    float fLength = node.fEnergy * Config::StickyBomb_fCloudVelocity;
    Vector3D vMove = node.vDir * fLength;
    dpoint3d pos = ConvertToVoxlap<dpoint3d>(node.vPos);
    dpoint3d move = ConvertToVoxlap<dpoint3d>(vMove);
    clipmove(&pos, &move, node.fRadius);

    if(vx5.cliphitnum > 0)
    {
      Vector3D vHitPos = ConvertFromVoxlap<dpoint3d>(pos);
      Vector3D vClippedMove = vHitPos - node.vPos;
      Vector3D vHitNormal = Normalize(vClippedMove - vMove);
      Voxlap::DrawVoxelExplosionHole(vHitPos, Config::StickyBomb_fCloudStainRadius * node.fEnergy);
      if(node.fEnergy > Config::StickyBomb_fCloudHoleEnergy)
      {
        for(int j = 0; j < Config::StickyBomb_iCloudHoleAmount; j++)
        {
          Vector3D vRand(-0.5f + rand()/(float)RAND_MAX, -0.5f + rand()/(float)RAND_MAX, -0.5f + rand()/(float)RAND_MAX);
          Voxlap::ClearVoxelSphere(vHitPos + vRand * Config::StickyBomb_fCloudHoleSpread * node.fEnergy, Config::StickyBomb_fCloudHoleRadius * node.fEnergy);
        }
      }

      node.vPos = vHitPos;
      // reflection
      node.vDir = Normalize(node.vDir - 2 * vHitNormal * Dot(node.vDir, vHitNormal));
      node.fEnergy *= Config::StickyBomb_fCloudCollisionEnergyDecay;
    }
    else
    {
      node.vPos += vMove;
    }

    node.pParticle->Start(node.vPos, node.fRadius, node.fEnergy);
    if(node.fRadius < Config::StickyBomb_fCloudMaxRadius)
      node.fRadius *= Config::StickyBomb_fCloudExpansion;
     i++;

    // damage entities
    {
      std::vector<Entity *> Entities;
      float fDamageRadius = node.fRadius;
      GetGame()->GetEntities(node.vPos - Vector3D(fDamageRadius,fDamageRadius,fDamageRadius),
                            node.vPos + Vector3D(fDamageRadius,fDamageRadius,fDamageRadius),
                            Entities);
      for(unsigned int e = 0; e < Entities.size(); e++)
      {
        Entity *pEntity = Entities[e];

        Vector3D vHitDir = pEntity->m_vPosition - node.vPos;
        float fHitDist = vHitDir.Normalize();

        Entity::DamageInfo dmg;
        dmg.Type = Entity::DamageType_HighExplosive;
        dmg.iAmount = Config::StickyBomb_fCloudEntityDamage * node.fEnergy;
        dmg.vHitPos = node.vPos;
        dmg.vHitNormal = -vHitDir;
        dmg.vHitDir = vHitDir;
        dmg.pHitSprite = NULL;
        pEntity->ReceiveDamage(dmg, m_pShooter);
      }
    }
  }
}


void StickyBombEntity::Tick(void)
{
  m_iTime++;

  if(!m_bDetonated)
  {
    if(m_iTime % (Game::iTicksPerSecond/2) == 0)
    {
      // tick sound
      Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/c4_tick.ogg");
      pSound->SetRange(20.0f, 700.0f);
      pSound->Play(m_vPosition, 0.95f + 0.05f * rand()/(float)RAND_MAX);
    }

    if(m_iTime > Config::StickyBomb_fDetonationDelay * Game::fTicksPerSecond)
    {
      Detonate();
    }
  }
  else
  {
    TickExplosions();

    if(m_Explosions.empty())
      GetGame()->RemoveEntity(this);
  }
}