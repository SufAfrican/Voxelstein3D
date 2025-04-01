#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// ExplosionCloud particle system
class ExplosionCloudParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_ExplosionCloud;
  ExplosionCloudParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin, float fRadius, float fEnergy);
  /// Renders the particles and updates the system
  void Render(float fDeltaTime);

private:
  /// Explosion origin
  Vector3D m_vOrigin;
  /// Explosion radius
  float m_fRadius;
  /// Explosion energy
  float m_fEnergy;
};