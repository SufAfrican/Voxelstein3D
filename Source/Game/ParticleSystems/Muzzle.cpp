#include "StdAfx.h"
#include "Game/ParticleSystems/Muzzle.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

MuzzleParticleSystem::MuzzleParticleSystem()
{
}


void MuzzleParticleSystem::Start(const Vector3D &vOrigin, const Vector3D &vNormal, int iSmokeAmount, float fMuzzleSizeFactor, float fSmokeSizeFactor, const char *strMuzzle)
{
  m_fTime = 0.0f;

  m_vPos = ConvertToVoxlap<Vector3D>(vOrigin);

  // make smoke particles
  Particle particle;
  particle.vPos = m_vPos;
  particle.vVel = Vector3D(0,0,0);
  particle.iAlpha = 255;
  m_Smoke.resize(iSmokeAmount, particle);
  for(unsigned int i = 0; i < m_Smoke.size(); i++)
  {
    m_Smoke[i] = particle;
    m_Smoke[i].vVel = 50.0f * (vNormal + 0.5f * Vector3D(-0.5f + rand() / (float)RAND_MAX, -0.5f + rand() / (float)RAND_MAX, -0.5f + rand() / (float)RAND_MAX));
  }

  m_pMuzzleImage = Voxlap::LoadImage(strMuzzle);
  m_pSmokeImage = Voxlap::LoadImage("png/smoke.png");

  m_fMuzzleSizeFactor = fMuzzleSizeFactor;
  m_fSmokeSizeFactor = fSmokeSizeFactor;
}

void MuzzleParticleSystem::Render(float fDeltaTime)
{
  // smoke puffs
  int iColor = (80)<<16 | (80)<<8 | (80);
  for(unsigned int i = 0; i < m_Smoke.size(); i++)
  {
    Particle &particle = m_Smoke[i];
    // slow it down
    particle.vVel -= (particle.vVel * Min(0.8f * fDeltaTime, 1.0f));
    // calculate new pos
    particle.vPos += particle.vVel * fDeltaTime;

    // lower alpha over time
    particle.iAlpha = 255 * Max(1.0f - m_fTime / 0.3f, 0.0f);

    // make it larger over time
    int iSize = m_fSmokeSizeFactor * Max((255 - particle.iAlpha) * 200000.0f / 255.0f, 100000.0f);

    if(particle.iAlpha > 0)
    {
      float x,y,z;
      if(project2d(particle.vPos.x, particle.vPos.y, particle.vPos.z, &x, &y, &z) == 1)
        drawtile(m_pSmokeImage->pData, m_pSmokeImage->iBytesPerLine, m_pSmokeImage->iWidth, m_pSmokeImage->iHeight, m_pSmokeImage->iWidth<<15, m_pSmokeImage->iHeight<<15,
        ((long)x)<<16, ((long)y)<<16, iSize*z, iSize*z, 0, (particle.iAlpha << 24) | iColor);
    }
  }

  // flash
  if(m_fTime < 0.05f)
  {
    float x,y,z;
    int iSize = m_fMuzzleSizeFactor * 65536 * 8 * 0.5f * (1.0f - Clamp(m_fTime / 0.05f, 0.0f, 1.0f) * 0.5f);
    if(project2d(m_vPos.x, m_vPos.y, m_vPos.z, &x, &y, &z) == 1)
      drawtile(m_pMuzzleImage->pData, m_pMuzzleImage->iBytesPerLine, m_pMuzzleImage->iWidth, m_pMuzzleImage->iHeight, m_pMuzzleImage->iWidth<<15, m_pMuzzleImage->iHeight<<15,
      ((long)x)<<16, ((long)y)<<16, iSize, iSize, 0, -1);
  }

  // end
  m_fTime += fDeltaTime;
  if(m_fTime > 0.3f)
  {
    Finish();
  }
}
