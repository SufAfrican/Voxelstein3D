#include "StdAfx.h"
#include "Game/ParticleSystems/ExplosionCloud.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

ExplosionCloudParticleSystem::ExplosionCloudParticleSystem()
{
}


void ExplosionCloudParticleSystem::Start(const Vector3D &vOrigin, float fRadius, float fEnergy)
{
  m_vOrigin = ConvertToVoxlap<Vector3D>(vOrigin);
  m_fRadius = fRadius;
  m_fEnergy = fEnergy;
  m_fTime = 0.0f;
}

void ExplosionCloudParticleSystem::Render(float fDeltaTime)
{
  Vector3D vFront = ConvertToVoxlap<Vector3D>(GetGame()->GetCameraMatrix().GetFrontVector());

  int iR = 255 * Min(m_fEnergy * 3.0f, 1.0f);
  int iG = 255 * Min(m_fEnergy * 2.0f, 1.0f);
  drawspherefill(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, -m_fRadius, 0xFF000000 | iR<<16);
  drawspherefill(m_vOrigin.x - vFront.x, m_vOrigin.y - vFront.y, m_vOrigin.z - vFront.z, -m_fRadius * 0.75f, 0xFF000000 | iR<<16 | iG<<8);
}
