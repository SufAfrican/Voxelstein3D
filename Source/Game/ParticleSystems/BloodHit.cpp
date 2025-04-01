#include "StdAfx.h"
#include "Game/ParticleSystems/BloodHit.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

BloodHitParticleSystem::BloodHitParticleSystem()
{
}

void BloodHitParticleSystem::Start(const Vector3D &vOrigin, const Vector3D &vNormal)
{
  // make debris particles
  Particle particle1;
  particle1.vPos = ConvertToVoxlap<Vector3D>(vOrigin);
  particle1.vVel = Vector3D(0,0,0);
  particle1.iColor = 0xFFFF0000;
  particle1.iAlive = 100;
  m_Drops.resize(10, particle1);
  for(unsigned int i = 0; i < m_Drops.size(); i++)
  {
    Particle &particle = m_Drops[i];
    particle = particle1;

    int iR = 50 + rand()%70;
    particle.iColor = 0xFF000000 | iR<<16;
    particle.vVel.x = (-0.5f + rand()/(float)RAND_MAX) * 100.0f;
    particle.vVel.y = (-0.5f + rand()/(float)RAND_MAX) * 100.0f;
    particle.vVel.z = (-0.5f + rand()/(float)RAND_MAX) * 500.0f;
    particle.vVel *= (i+1)/(float)m_Drops.size();
  }
}

void BloodHitParticleSystem::Render(float fDeltaTime)
{
  bool bStillMoving = false;

  // blood drops
  for(int i = 0; i < m_Drops.size(); i++)
  {
    Particle &particle = m_Drops[i];
    if(particle.iAlive <= 0)
      continue;
    else
      bStillMoving = true;

    float fFactor = Min(m_fTime + i/(float)m_Drops.size(), 1.0f);
    particle.vVel.z += 300.0f * fDeltaTime;
    Vector3D vOldPos = particle.vPos;
    particle.vPos += particle.vVel * fDeltaTime * fFactor;
    drawspherefill(particle.vPos.x, particle.vPos.y, particle.vPos.z, -0.5f, particle.iColor);

    Voxlap::HitInfo info;
    Vector3D vOldPos2 = ConvertFromVoxlap(vOldPos);
    Vector3D vNewPos = ConvertFromVoxlap(particle.vPos);
    Vector3D vDir = vNewPos - vOldPos2;
    float fLength = vDir.Normalize();
    if(Voxlap::HitScan(vOldPos2, vDir, fLength, info))
    {
      int iStrength = Min((particle.iAlive*255)/100 + rand()%50, 255);

      if(fabsf(info.vNormal.y) < 0.001f)
      {
        Vector3D vNormal = ConvertToVoxlap<Vector3D>(info.vNormal);
        particle.vPos.x = LerpFloat(particle.vPos.x, vOldPos.x, fabsf(vNormal.x));
        particle.vPos.y = LerpFloat(particle.vPos.y, vOldPos.y, fabsf(vNormal.y));
        particle.vPos.z = LerpFloat(particle.vPos.z, vOldPos.z, fabsf(vNormal.z));
        // move one voxel per second
        particle.vVel.Normalize();
        // move one voxel per frame
        particle.vVel /= fDeltaTime;
        particle.iAlive -= 10;
      }
      else
      {
        particle.vPos = vOldPos;
        particle.vPos.x += -1 + 2*(rand()%2);
        particle.vPos.y += -1 + 2*(rand()%2);
        particle.iAlive -= 30;
      }

      Voxlap::DrawVoxelBloodSplat(info.vPos, iStrength);
    }
  }

  // end
  m_fTime += fDeltaTime;
  if(!bStillMoving || m_fTime > 5.0f)
  {
    Finish();
  }
}
