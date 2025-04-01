#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// Blood hit effect
class BloodHitParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_BloodHit;
  BloodHitParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin, const Vector3D &vNormal);
  /// Renders the particles
  void Render(float fDeltaTime);

private:
  /// Particle data
  struct Particle
  {
    Vector3D vPos;
    Vector3D vVel;
    long iColor;
    int iAlive;
  };
  /// Blood drop particles
  std::vector<Particle> m_Drops;
};