#include "StdAfx.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/ParticleSystem.h"

ParticleSystemManager *g_pParticleSystemManager = NULL;

ParticleSystemManager::~ParticleSystemManager()
{
  ParticleMap::iterator it;
  for(it = m_ParticleMap.begin(); it != m_ParticleMap.end(); it++)
    delete it->second;
}

ParticleSystem *ParticleSystemManager::GetRecyclableSystem(int iType, unsigned int iMaxActiveCount)
{
  // find reusable one
  ParticleMap::iterator it = m_ParticleMap.find(iType);
  if(it != m_ParticleMap.end())
  {
    unsigned int iActiveCount = 0;
    ParticleSystem *pOldest = it->second;

    while(it != m_ParticleMap.end() && it->first == iType)
    {
      // return finished one
      if(it->second->IsFinished())
        return it->second;
      
      // get oldest
      if(it->second->GetTime() > pOldest->GetTime())
        pOldest = it->second;

      // count active ones
      iActiveCount++;
      it++;
    }

    // too many active - return the oldest one
    if(iActiveCount > iMaxActiveCount)
    {
      pOldest->Finish();
      return pOldest;
    }
  }
  return NULL;
}

void ParticleSystemManager::GetNumSystems(int iType, unsigned int &iNumTotal, unsigned int &iRecyclable)
{
  iNumTotal = 0;
  iRecyclable = 0;
  ParticleMap::iterator it = m_ParticleMap.find(iType);
  if(it != m_ParticleMap.end())
  {
    while(it != m_ParticleMap.end() && it->first == iType)
    {
      iNumTotal++;
      if(it->second->IsFinished())
        iRecyclable++;
      it++;
    }
  }
}

void ParticleSystemManager::AddRecyclableSystem(ParticleSystem *pSystem, int iType)
{
  m_ParticleMap.insert( std::pair<int, ParticleSystem *>(iType, pSystem) );
}

void ParticleSystemManager::Update(void)
{
}