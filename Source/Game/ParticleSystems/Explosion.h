#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// Explosion particle system
class ExplosionParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_Explosion;
  ExplosionParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin, float fRadius, int iSparks = 50);
  /// Renders the particles and updates the system
  void Render(float fDeltaTime);

private:
  /// Particle data
  struct Particle
  {
    Vector3D vPos;
    Vector3D vVel;
    long iColor;
    float fUnique;
  };
  /// Spark particles
  std::vector<Particle> m_Sparks;

  /// Explosion origin
  Vector3D m_vOrigin;
  /// Explosion radius
  float m_fRadius;
};