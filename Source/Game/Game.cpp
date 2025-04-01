#include "StdAfx.h"
#include "Game/Entities/Entity.h"
#include "Game/Entities/EntityTree.h"
#include "Game/Entities/PlayerEntity.h"
#include "Game/Entities/SoldierEntity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Engine/Sound/SoundDevice.h"

const float Game::fTicksPerSecond = 50.0f;
const unsigned int Game::iTicksPerSecond = 50;

Game::Game()
{
  m_mCamera = Matrix44::mIdentity;
  m_vMouse = Vector2D(0,0);

  m_fTickDuration = 1.0f / fTicksPerSecond;
  m_fTime = Time::GetTime();
  m_fLastTick = m_fTime;

  m_pEntityTree = new EntityTree();
  g_pPhysicsSolver = new PhysicsSolver();
  g_pParticleSystemManager = new ParticleSystemManager();

  memset(m_bKeyDown, 0, sizeof(bool) * Key_NumKeys);

  m_iCurrentTick = 0;

  m_bFlashLight = false;
  m_vFlashLightPos = Vector3D(0,0,0);
  m_bTicking = false;
  m_bLevelFinished = false;
}

Game::~Game()
{
  while(!m_Entities.empty())
    RemoveEntity(m_Entities.front());
  while(!m_RemovedEntities.empty())
    delete m_RemovedEntities.front();
  delete m_pEntityTree;
  delete g_pPhysicsSolver;
  delete g_pParticleSystemManager;
}

void Game::CreatePlayer(const Vector3D &vPos)
{
  m_pLocalPlayer = new PlayerEntity();
  m_pLocalPlayer->m_vPosition = vPos;
  m_pLocalPlayer->m_vRotation.x = -0.5f * Math::fPi;
  AddEntity(m_pLocalPlayer);
}

void Game::AddEntity(Entity *pEntity)
{
  // add reference
  pEntity->AddReference();
  m_Entities.push_back(pEntity);
  m_pEntityTree->AddEntity(pEntity);
}

void Game::RemoveEntity(Entity *pEntity)
{
  // cannot modify entity list while ticking, so just queue it
  if(m_bTicking)
  {
    m_EntitiesPendingRemoval.push_back(pEntity);
    return;
  }

  // find entity
  for(unsigned int i = 0; i < m_Entities.size(); i++)
  {
    if(m_Entities[i] == pEntity)
    {
      // pending deletion..
      m_RemovedEntities.push_back(pEntity);

      // remove
      m_Entities[i] = m_Entities.back();
      m_Entities.pop_back();
      m_pEntityTree->RemoveEntity(pEntity);

      // remove reference
      pEntity->RemoveReference();
      return;
    }
  }

}

void Game::OnEntityDelete(Entity *pEntity)
{
  for(unsigned int i = 0; i < m_RemovedEntities.size(); i++)
  {
    if(m_RemovedEntities[i] == pEntity)
    {
      m_RemovedEntities[i] = m_RemovedEntities.back();
      m_RemovedEntities.pop_back();
      return;
    }
  }
  printf("Game::OnEntityDelete - entity was not removed before deleting!");
}

void Game::Run(void)
{
  m_fTime = Time::GetTime();

  int iSafety = 0;
  while(m_fTime - m_fLastTick >= m_fTickDuration)
  {
    m_iCurrentTick++;
    m_bTicking = true;

    for(unsigned int i = 0; i < m_Entities.size(); i++)
    {
      Entity *pEntity = m_Entities[i];
      pEntity->Tick();

      // update entity tree
      if(pEntity->m_iFlags & Entity::Flag_HasMoved)
      {
        m_pEntityTree->UpdateEntity(pEntity);
        pEntity->m_iFlags &= ~(Entity::Flag_HasMoved);
      }

      // visualize
      if(Config::Debug_bVisualizeCollision && pEntity->m_CollisionType != Entity::Collision_None)
        Voxlap::AddDebugBoxLines(pEntity->m_vPosition - pEntity->m_vCollisionBox * 0.5f,
                                 pEntity->m_vPosition + pEntity->m_vCollisionBox * 0.5f, 255,0,0);
      if(Config::Debug_bVisualizeEntitySize)
        Voxlap::AddDebugBoxLines(pEntity->m_vPosition - pEntity->m_vSize * 0.5f,
                                 pEntity->m_vPosition + pEntity->m_vSize * 0.5f, 0,255,0);
    }

    // update tree
    if(m_iCurrentTick % Game::iTicksPerSecond == 0)
    {
      m_pEntityTree->ClearEmptyNodes();
    }

    // update physics
    g_pPhysicsSolver->Tick();

    // update particle effects
    g_pParticleSystemManager->Update();

    // update listener
    GetSoundDevice()->SetListener(m_mCamera);

    // update time
    m_fLastTick += m_fTickDuration;
    m_bTicking = false;

    // safely remove entities
    for(unsigned int i = 0; i < m_EntitiesPendingRemoval.size(); i++)
      RemoveEntity(m_EntitiesPendingRemoval[i]);
    m_EntitiesPendingRemoval.clear();

    // mouse delta fix for low FPS
    m_vMouse = Vector2D(0,0);

    // check if game was hanging
    iSafety++;
    if(iSafety > 10)
    {
      // let's just skip whatever time has passed...
      m_fLastTick = m_fTime;
      break;
    }
  }
}

void Game::GetEntities(int iClass, std::vector<Entity *> &out)
{
  for(unsigned int i = 0; i < m_Entities.size(); i++)
  {
    Entity *pEntity = m_Entities[i];
    if(pEntity->IsOfClass(iClass))
      out.push_back(pEntity);
  }
}

void Game::GetEntities(const Vector3D &vStart, const Vector3D &vEnd, std::vector<Entity *> &out)
{
  m_pEntityTree->GetEntities(vStart, vEnd, out);
}

Vector3D Game::ClipMove(const Vector3D &vPosition, const Vector3D &vCollisionBox, const Vector3D &vMotion, unsigned int iCollisionTypeMask)
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
  GetEntities(vMin, vMax, Entities);

  // test all entities
  Vector3D vSweepDir = vMotion;
  float fSweepDist = vSweepDir.Normalize();
  float fMinHitDist = fSweepDist;
  bool bAxisBlocked[3] = {false,false,false};
  for(unsigned int i = 0; i < Entities.size(); i++)
  {
    Entity *pEntity = Entities[i];

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

Game::Stats::Stats()
{
  iVoxels = 0;
  iEnemies = 0;
  iTreasures = 0;
  iItems = 0;
}
