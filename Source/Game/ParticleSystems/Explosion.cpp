#include "StdAfx.h"
#include "Game/ParticleSystems/Explosion.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

ExplosionParticleSystem::ExplosionParticleSystem()
{
}


void ExplosionParticleSystem::Start(const Vector3D &vOrigin, float fRadius, int iSparks)
{
  m_vOrigin = ConvertToVoxlap<Vector3D>(vOrigin);
  m_fRadius = fRadius;

  // make spark particles
  Particle particle1;
  particle1.vPos = ConvertToVoxlap<Vector3D>(vOrigin);
  particle1.vVel = Vector3D(0,0,0);
  particle1.iColor = 0xFFFFFF50;
  m_Sparks.resize(iSparks, particle1);
  for(unsigned int i = 0; i < m_Sparks.size(); i++)
  {
    Particle &particle = m_Sparks[i];
    particle = particle1;
    particle.vVel.x = -0.5f + rand()/(float)RAND_MAX;
    particle.vVel.y = -0.5f + rand()/(float)RAND_MAX;
    particle.vVel.z = -0.5f + rand()/(float)RAND_MAX;
    particle.vVel.Normalize();
    particle.vPos += particle.vVel * 0.75f * fRadius;
    particle.vVel *= 500.0f;
    particle.fUnique = 0.5f + rand()/(float)RAND_MAX * 0.5f;
  }
}

void ExplosionParticleSystem::Render(float fDeltaTime)
{
  bool bStillMoving = false;

  if(m_fTime < 0.1f)
  {
    bStillMoving = true;
    float fFac = Max((m_fTime / 0.1f), 0.5f);

    Vector3D vFront = ConvertToVoxlap<Vector3D>(GetGame()->GetCameraMatrix().GetFrontVector());

    drawspherefill(m_vOrigin.x, m_vOrigin.y, m_vOrigin.z, -m_fRadius * fFac * 1.5f, 0xFFFF0000);
    drawspherefill(m_vOrigin.x - vFront.x, m_vOrigin.y - vFront.y, m_vOrigin.z - vFront.z, -m_fRadius * fFac, 0xFFFFFF00);
  }
  else
  {
    // sparks
    float fFactor = 1.0f - Min(m_fTime/5.0f, 1.0f);
    float fFactor2 = 1.0f - Min(m_fTime/1.0f, 1.0f);
    for(int i = 0; i < m_Sparks.size(); i++)
    {
      Particle &particle = m_Sparks[i];
      bool bMoving = particle.vVel.SquaredLength() > 5.0f * 5.0f;
      if(bMoving)
        bStillMoving = true;
      Vector3D vOldPos = particle.vPos;
      particle.vVel.z += 400.0f * fDeltaTime;
      particle.vPos += particle.vVel * fDeltaTime;
      int iR = (particle.iColor>>16) & 0xFF;
      int iG = (particle.iColor>>8) & 0xFF;
      int iB = (particle.iColor) & 0xFF;
      iR *= fFactor * particle.fUnique;
      iG *= fFactor2 * particle.fUnique;
      iB *= fFactor2 * particle.fUnique;
      int iColor = (iR<<16) | (iG<<8) | (iB);
      if(!bMoving)
        drawspherefill(particle.vPos.x, particle.vPos.y, particle.vPos.z, -0.4f, iColor);
      else
        drawline3d(vOldPos.x, vOldPos.y, vOldPos.z,
          particle.vPos.x, particle.vPos.y, particle.vPos.z, iColor);

      // particle collision
      if(bMoving)
      {
        Voxlap::HitInfo info;
        Vector3D vOldPos2 = ConvertFromVoxlap(vOldPos);
        Vector3D vNewPos = ConvertFromVoxlap(particle.vPos);
        Vector3D vDir = vNewPos - vOldPos2;
        float fLength = vDir.Normalize();
        if(Voxlap::HitScan(vOldPos2, vDir, fLength, info))
        {
          if(particle.vVel.SquaredLength() > 300.0f * 300.0f)
            Voxlap::DrawVoxelBulletHole(info.vPos);

          particle.vPos = ConvertToVoxlap<Vector3D>(info.vPos);
          Vector3D vNormal = ConvertToVoxlap<Vector3D>(info.vNormal);

          float fVelLen = particle.vVel.Normalize();
          // reflection v = i - 2 * n * dot(i•n)
          particle.vVel = particle.vVel - 2 * vNormal * Dot(particle.vVel, vNormal);
          Normalize(particle.vVel);
          particle.vVel *= fVelLen * 0.5f;
        }
      }
    }
  }

  // end
  m_fTime += fDeltaTime;
  if(!bStillMoving || m_fTime > 5.0f)
  {
    Finish();
  }
}
