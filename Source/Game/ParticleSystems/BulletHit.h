#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// Bullet hit effect
class BulletHitParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_BulletHit;
  BulletHitParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin, const Vector3D &vNormal, int iColor);
  /// Renders the particles and updates the system
  void Render(float fDeltaTime);

private:
  /// Particle data
  struct Particle
  {
    Vector3D vPos;
    Vector3D vVel;
    long iColor;
    int iAlpha;
  };
  /// Debris particles
  std::vector<Particle> m_Debris;
  /// Smoke particles
  std::vector<Particle> m_Smoke;

  /// Spark start location
  Vector3D m_vSparkPos;
  /// Spark directions
  std::vector<Vector3D> m_vSparkDirs;
  /// Smoke particle image
  VoxlapImage *m_pSmokeImage;
};