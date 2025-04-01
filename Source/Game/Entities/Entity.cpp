#include "StdAfx.h"
#include "Game/Entities/Entity.h"

Entity::Entity()
{
  m_vPosition = Vector3D(0,0,0);
  m_vRotation = Vector3D(0,0,0);
  m_vVelocity = Vector3D(0,0,0);
  m_vCollisionBox = Vector3D(0,0,0);
  m_CollisionType = Collision_None;
  m_vSize = Vector3D(0,0,0);
  m_pTreeNode = NULL;
  m_iFlags = 0;
  m_iReferenceCount = 0;
}

Entity::~Entity()
{
  GetGame()->OnEntityDelete(this);
}

void Entity::Tick(void)
{
}

Vector3D Entity::ClipMove(const Vector3D &vPosition, const Vector3D &vCollisionBox, const Vector3D &vMotion, unsigned int iCollisionTypeMask)
{
  Vector3D vCollisionHalfBox = vCollisionBox * 0.5f;

  // get all entities in sweep area
  Vector3D vMin, vMax;
  Vector3D vEnd = vPosition + vMotion;
  for(int i = 0; i < 3; i++)
  {
    vMin[i] = Min(vPosition[i], vEnd[i]);
    vMax[i] = Max(vPosition[i], vEnd[i]);
  }
  vMin -= vCollisionHalfBox;
  vMax += vCollisionHalfBox;
  std::vector<Entity *> Entities;
  GetGame()->GetEntities(vMin, vMax, Entities);


  // test all entities
  Vector3D vSweepDir = vMotion;
  float fSweepDist = vSweepDir.Normalize();
  float fMinHitDist = fSweepDist;
  bool bAxisBlocked[3] = {false,false,false};
  for(unsigned int i = 0; i < Entities.size(); i++)
  {
    Entity *pEntity = Entities[i];

    // ignore self
    if(this == pEntity)
      continue;

    // ignore collision type
    if(!(pEntity->m_CollisionType & iCollisionTypeMask))
      continue;

    float fHitDist;
    int iHitAxis;
    if(SweepBoxBox(vPosition, vCollisionHalfBox, pEntity->m_vPosition, pEntity->m_vCollisionBox * 0.5f, vSweepDir, fHitDist, iHitAxis))
    {
      if(fHitDist < fSweepDist)
      {
        // get nearest distance
        if(fHitDist < fMinHitDist)
          fMinHitDist = fHitDist;

        // block axis
        bAxisBlocked[iHitAxis] = true;
      }
    }
  }

  // make it possible to move along non-blocked axis
  Vector3D vBlockedMotion = vSweepDir * fMinHitDist;
  Vector3D vClampedMotion = vMotion;
  for(int i = 0; i < 3; i++)
  {
    if(bAxisBlocked[i])
      vClampedMotion[i] = vBlockedMotion[i];
  }

  return vPosition + vClampedMotion;
}

bool Entity::HitScanThis(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info)
{
  // no collision
  if(m_CollisionType == Collision_None)
    return false;

  // test bounding box
  if(!TestRayBox(vPos, vDir, fDistance, m_vPosition, m_vSize * 0.5f))
    return false;

  // no collision sprites
  if(m_CollisionSprites.empty())
  {
    // test box intersection
    int iHitAxis = 0;
    float fHitDistance = FLT_MAX;
    if(IntersectRayBox(vPos, vDir, m_vPosition, m_vCollisionBox * 0.5f, fHitDistance, iHitAxis)
      && fHitDistance <= fDistance)
    {
      Vector3D vHitNormal = Vector3D(0,0,0);
      vHitNormal[iHitAxis] = -Sign(vDir[iHitAxis]);
      info.fDistance = fHitDistance;
      info.vNormal = vHitNormal;
      info.iColor = 0;
      info.vPos = vPos + vDir * fHitDistance;
      info.pEntity = this;
      info.pSprite = NULL;
      return true;
    }
  }
  else
  {
    // find nearest hit
    float fHitDistance = FLT_MAX;

    // test sprite collisions
    for(unsigned int iSprite = 0; iSprite < m_CollisionSprites.size(); iSprite++)
    {
      Voxlap::HitInfo voxinfo;
      if(Voxlap::HitScan(m_CollisionSprites[iSprite], vPos, vDir, fDistance, voxinfo)
        && voxinfo.fDistance < fHitDistance)
      {
        *((Voxlap::HitInfo *)&info) = voxinfo;
        info.pSprite = m_CollisionSprites[iSprite];
        fHitDistance = voxinfo.fDistance;
      }
    }

    if(fHitDistance < FLT_MAX)
    {
      info.pEntity = this;
      return true;
    }
  }

  info.pEntity = NULL;
  info.pSprite = NULL;
  return false;
}

bool Entity::HitScan(const Vector3D &vPos, const Vector3D &vDir, float fDistance, HitInfo &info, bool bTestWorld, unsigned int iCollisionTypeMask)
{
  // test world
  Voxlap::HitInfo WorldInfo;
  bool bHitWorld = false;
  if(bTestWorld)
  {
    bHitWorld = Voxlap::HitScan(vPos, vDir, fDistance, WorldInfo);
    if(bHitWorld)
    {
      // clamp distance as an optimization
      fDistance = WorldInfo.fDistance;
    }
  }

  // get all entities in ray area
  Vector3D vMin, vMax;
  Vector3D vEnd = vPos + vDir * fDistance;
  for(int i = 0; i < 3; i++)
  {
    vMin[i] = Min(vPos[i], vEnd[i]);
    vMax[i] = Max(vPos[i], vEnd[i]);
  }
  std::vector<Entity *> Entities;
  GetGame()->GetEntities(vMin, vMax, Entities);

  info.pEntity = NULL;
  info.pSprite = NULL;

  // test all entities
  Entity *pClosestHit = NULL;
  float fClosestHit = fDistance;
  for(unsigned int i = 0; i < Entities.size(); i++)
  {
    Entity *pEntity = Entities[i];

    // ignore self
    if(this == pEntity)
      continue;

    // ignore collision type
    if(!(pEntity->m_CollisionType & iCollisionTypeMask))
      continue;    

    // do hitscan
    Entity::HitInfo info2;
    if(pEntity->HitScanThis(vPos, vDir, fDistance, info2))
    {
      // get closest hit
      if(info2.fDistance < fClosestHit)
      {
        info = info2;
        fClosestHit = info2.fDistance;
        pClosestHit = pEntity;
      }
    }
  }

  // don't test world
  if(!bTestWorld)
    return pClosestHit != NULL;

  // hit world and entity
  if(bHitWorld && pClosestHit)
  {
    // world hit is closer
    if(WorldInfo.fDistance < fClosestHit)
    {
      *((Voxlap::HitInfo *)&info) = WorldInfo;
      info.pEntity = NULL;
      info.pSprite = NULL;
    }
    return true;
  }
  // hit world only
  else if(bHitWorld && !pClosestHit)
  {
    *((Voxlap::HitInfo *)&info) = WorldInfo;
    info.pEntity = NULL;
    info.pSprite = NULL;
    return true;
  }
  // hit entity only
  else if(pClosestHit)
  {
    return true;
  }

  return false;
}

// todo: fix this
#include "Game/Entities/SoldierEntity.h"
void Entity::SendSoundEvent(const Vector3D &vPos, float fRadius)
{
  std::vector<Entity *> Entities;
  GetGame()->GetEntities(vPos - Vector3D(fRadius,fRadius,fRadius), vPos + Vector3D(fRadius,fRadius,fRadius), Entities);
  for(unsigned int i = 0; i < Entities.size(); i++)
  {
    if(Entities[i]->IsOfClass(Entity::Class_Soldier))
    {
      ((SoldierEntity *)Entities[i])->OnHearSound(vPos, this);
    }
  }
}

// todo: fix this
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/BulletHit.h"
void DamageSprite(Sprite *pSprite, const Entity::DamageInfo &Damage, bool bPlaySound)
{
  Voxlap::DrawVoxelBulletHole(pSprite, Damage.vHitPos);

  if(Damage.Type == Entity::DamageType_Bullet)
  {
    GetParticles()->CreateSystem<BulletHitParticleSystem>()->Start(Damage.vHitPos, Damage.vHitNormal, 0);

    if(bPlaySound)
    {
      float fPitch = 0.8f + 0.4f * rand()/(float)RAND_MAX;
      Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/hit.ogg");
      pSound->SetRange(10.0f, 200.0f);
      pSound->Play(Damage.vHitPos, fPitch);
    }
  }
}
