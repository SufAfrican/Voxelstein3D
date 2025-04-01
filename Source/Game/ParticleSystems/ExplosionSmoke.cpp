#include "StdAfx.h"
#include "Game/ParticleSystems/ExplosionSmoke.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

ExplosionSmokeParticleSystem::ExplosionSmokeParticleSystem()
{
}


void ExplosionSmokeParticleSystem::Start(const Vector3D &vOrigin, const Vector3D &vVel, float fRadius, float fBrightness)
{
  m_vOrigin = ConvertToVoxlap<Vector3D>(vOrigin);
  m_vVelocity = ConvertToVoxlap<Vector3D>(vVel);
  m_fRadius = fRadius;
  m_fBrightness = Clamp(fBrightness, 0.0f, 1.0f);
  m_fTime = 0.0f;
  m_fShrinkage = 2.0f + 1.0f * rand()/(float)RAND_MAX;
}

void ExplosionSmokeParticleSystem::Render(float fDeltaTime)
{
  Vector3D vFront = ConvertToVoxlap<Vector3D>(GetGame()->GetCameraMatrix().GetFrontVector());

  int iR = 255 * m_fBrightness;
  drawspherefill(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, -m_fRadius, 0xFF000000 | iR<<16 | iR<<8 | iR);

  m_vOrigin += m_vVelocity * fDeltaTime;
  m_vVelocity *= 0.95f;
  m_vVelocity.z -= 8.0f * fDeltaTime;

  m_fTime += fDeltaTime;

  m_fRadius -= m_fShrinkage * fDeltaTime;
  if(m_fRadius < 10.0f)
    Finish();
}
