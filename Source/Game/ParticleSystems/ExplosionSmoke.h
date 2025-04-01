#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// Explosion smoke particle system
class ExplosionSmokeParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_ExplosionSmoke;
  ExplosionSmokeParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin, const Vector3D &vVel, float fRadius, float fBrightness);
  /// Renders the particles and updates the system
  void Render(float fDeltaTime);

private:
  /// Smoke origin
  Vector3D m_vOrigin;
  /// Smoke velocity
  Vector3D m_vVelocity;
  /// Smoke radius
  float m_fRadius;
  /// Smoke brightness
  float m_fBrightness;
  /// Shrinking speed
  float m_fShrinkage;
};