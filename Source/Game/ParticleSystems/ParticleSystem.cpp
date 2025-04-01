#include "StdAfx.h"
#include "Game/ParticleSystems/ParticleSystem.h"

extern void RegisterParticleSystem(ParticleSystem *pSystem);
extern void UnregisterParticleSystem(ParticleSystem *pSystem);

ParticleSystem::ParticleSystem()
{
  m_fTime = 0.0f;
  m_bFinished = false;
}

ParticleSystem::~ParticleSystem()
{
  // just in case
  UnregisterParticleSystem(this);
}

void ParticleSystem::PrepareStart(void)
{
  m_fTime = 0.0f;
  m_bFinished = false;
  RegisterParticleSystem(this);
}

void ParticleSystem::Finish(void)
{
  UnregisterParticleSystem(this);
  m_bFinished = true;
}