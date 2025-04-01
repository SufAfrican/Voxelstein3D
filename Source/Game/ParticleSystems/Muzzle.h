#pragma once

#include "Game/ParticleSystems/ParticleSystem.h"

/// Muzzle flash effect
class MuzzleParticleSystem : public ParticleSystem
{
public:
  static const ParticleSystemClass m_Class = Class_Muzzle;
  MuzzleParticleSystem();

  /// Starts or restarts the system
  void Start(const Vector3D &vOrigin,
      const Vector3D &vNormal,
      int iSmokeAmount = 2,
      float fMuzzleSizeFactor = 1.0f,
      float fSmokeSizeFactor = 1.0f,
      const char *strMuzzle = "png/muzzle.png");

  /// Renders the particles and updates the system
  void Render(float fDeltaTime);

private:
  /// Particle data
  struct Particle
  {
    Vector3D vPos;
    Vector3D vVel;
    int iAlpha;
  };
  /// Smoke particles
  std::vector<Particle> m_Smoke;

  /// Time in seconds
  float m_fTime;
  /// Total duration of effect in seconds
  float m_fLifeTime;
  /// Effect position
  Vector3D m_vPos;

  /// Muzzle size factor
  float m_fMuzzleSizeFactor;
  /// Smoke size factor
  float m_fSmokeSizeFactor;

  /// Muzzle flash image
  VoxlapImage *m_pMuzzleImage;
  /// Smoke particle image
  VoxlapImage *m_pSmokeImage;
};
