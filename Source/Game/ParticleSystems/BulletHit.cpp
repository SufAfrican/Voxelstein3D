#include "StdAfx.h"
#include "Game/ParticleSystems/BulletHit.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

BulletHitParticleSystem::BulletHitParticleSystem()
{
}

void BulletHitParticleSystem::Start(const Vector3D &vOrigin, const Vector3D &vNormal, int iColor)
{
  // normal
  Vector3D vVoxlapNormal = ConvertToVoxlap<Vector3D>(vNormal);
  for(int i = 0; i < 3; i++)
  {
    if(fabsf(vVoxlapNormal[i]) < 0.0001f)
      vVoxlapNormal[i] = 0;
  }
  float fDirX = SignOrZero(vVoxlapNormal.x);
  float fDirY = SignOrZero(vVoxlapNormal.y);
  float fDirZ = SignOrZero(vVoxlapNormal.z);

  // get color values
  int iIntensity, iR, iG, iB;
  iIntensity = (iColor>>24) & 0xFF;
  iR = (iColor>>16) & 0xFF;
  iG = (iColor>>8) & 0xFF;
  iB = (iColor) & 0xFF;

  // make slightly darker
  iIntensity *= 9; iIntensity /= 10;

  // add intensity to color
  iR *= iIntensity; iR /= 128; if(iR > 255) iR = 255;
  iG *= iIntensity; iG /= 128; if(iG > 255) iG = 255;
  iB *= iIntensity; iB /= 128; if(iB > 255) iB = 255;

  // construct new color
  iColor = (0xFF<<24) | (iR<<16) | (iG<<8) | (iB);

  // make debris particles
  Particle particle1;
  particle1.vPos = ConvertToVoxlap<Vector3D>(vOrigin);
  particle1.vVel = Vector3D(0,0,0);
  particle1.iColor = iColor;
  particle1.iAlpha = 255;
  m_Debris.resize(5, particle1);
  for(unsigned int i = 0; i < m_Debris.size(); i++)
  {
    Particle &particle = m_Debris[i];
    particle = particle1;
    if(fDirX == 0)
      particle.vVel.x = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.x = fDirX * (0.75f + rand()/(float)RAND_MAX);

    if(fDirY == 0)
      particle.vVel.y = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.y = fDirY * (0.75f + rand()/(float)RAND_MAX);

    if(fDirZ == 0)
      particle.vVel.z = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.z = fDirZ * (0.75f + rand()/(float)RAND_MAX);

    particle.vVel *= 50.0f;
  }

  // make smoke particles
  // multiply smoke color by intensity
  iIntensity = Clamp(iIntensity, 60, 255);
  particle1.iColor = (iIntensity << 16) | (iIntensity << 8) | (iIntensity);
  m_Smoke.resize(3, particle1);
  for(unsigned int i = 0; i < m_Smoke.size(); i++)
  {
    Particle &particle = m_Smoke[i];
    particle = particle1;
    if(fDirX == 0)
      particle.vVel.x = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.x = fDirX * (0.5f + rand()/(float)RAND_MAX);

    if(fDirY == 0)
      particle.vVel.y = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.y = fDirY * (0.5f + rand()/(float)RAND_MAX);

    if(fDirZ == 0)
      particle.vVel.z = -0.5f + rand()/(float)RAND_MAX;
    else
      particle.vVel.z = fDirZ * (0.5f + rand()/(float)RAND_MAX);

    particle.vVel *= 10.0f;
  }


  m_fTime = 0.0f;

  m_vSparkPos = ConvertToVoxlap<Vector3D>(vOrigin);
  m_vSparkDirs.resize(4);
  for(unsigned int i = 0; i < m_vSparkDirs.size(); i++)
  {
    m_vSparkDirs[i] = vVoxlapNormal * 0.5f
                         + 0.5f * Vector3D(-0.5f + rand() / (float)RAND_MAX, -0.5f + rand() / (float)RAND_MAX, -0.5f + rand() / (float)RAND_MAX);

  }

  m_pSmokeImage = Voxlap::LoadImage("png/smoke.png");
}

void BulletHitParticleSystem::Render(float fDeltaTime)
{
  // debris puffs
  for(int i = 0; i < m_Debris.size(); i++)
  {
    Particle &particle = m_Debris[i];
    particle.vVel.z += 300.0f * fDeltaTime;
    particle.vPos += particle.vVel * fDeltaTime;
    drawspherefill(particle.vPos.x, particle.vPos.y, particle.vPos.z, -0.4f, particle.iColor);
  }

  // smoke puffs
  for(unsigned int i = 0; i < m_Smoke.size(); i++)
  {
    Particle &particle = m_Smoke[i];
    // slow it down
    particle.vVel -= (particle.vVel * Min(0.8f * fDeltaTime, 1.0f));
    // calculate new pos
    particle.vPos += particle.vVel * fDeltaTime;

    // lower alpha over time
    particle.iAlpha = 255 * Max(1.0f - m_fTime / 0.8f, 0.0f);

    // make it larger over time
    int iSize = Max((255 - particle.iAlpha) * 30000.0f / 255.0f, 10000.0f);

    if(particle.iAlpha > 0)
    {
      float x,y,z;
      if(project2d(particle.vPos.x, particle.vPos.y, particle.vPos.z, &x, &y, &z) == 1)
        drawtile(m_pSmokeImage->pData, m_pSmokeImage->iBytesPerLine, m_pSmokeImage->iWidth, m_pSmokeImage->iHeight, m_pSmokeImage->iWidth<<15, m_pSmokeImage->iHeight<<15,
        ((long)x)<<16, ((long)y)<<16, iSize*z, iSize*z, 0, (particle.iAlpha << 24) | (particle.iColor));
    }
  }

  // spark lines
  if(m_fTime < 0.1f)
  {
    for(int i = 0; i < m_vSparkDirs.size(); i++)
    {
      // calculate random direction
      float fFactor = Min(m_fTime/0.1f, 1.0f);
      Vector3D vSparkDir = fFactor * m_vSparkDirs[i];
      drawline3d(m_vSparkPos.x, m_vSparkPos.y, m_vSparkPos.z,
        m_vSparkPos.x + 1 * vSparkDir.x,
        m_vSparkPos.y + 1 * vSparkDir.y,
        m_vSparkPos.z + 1 * vSparkDir.z, 0x00FFFF00);
      drawline3d(
        m_vSparkPos.x + 1 * vSparkDir.x,
        m_vSparkPos.y + 1 * vSparkDir.y,
        m_vSparkPos.z + 1 * vSparkDir.z,
        m_vSparkPos.x + 2 * vSparkDir.x,
        m_vSparkPos.y + 2 * vSparkDir.y,
        m_vSparkPos.z + 2 * vSparkDir.z, 0x00FFA000);
      drawline3d(
        m_vSparkPos.x + 2 * vSparkDir.x,
        m_vSparkPos.y + 2 * vSparkDir.y,
        m_vSparkPos.z + 2 * vSparkDir.z,
        m_vSparkPos.x + 2.5f * vSparkDir.x,
        m_vSparkPos.y + 2.5f * vSparkDir.y,
        m_vSparkPos.z + 2.5f * vSparkDir.z, 0x00FF6000);
    }
  }

  // end
  m_fTime += fDeltaTime;
  if(m_fTime > 1.0f)
  {
    Finish();
  }
}
