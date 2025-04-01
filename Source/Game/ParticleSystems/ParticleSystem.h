#pragma once

/// Base class for a recyclable particle system
class ParticleSystem
{
public:
  /// Possible particle system classes
  enum ParticleSystemClass
  {
    Class_Muzzle,
    Class_BulletHit,
    Class_Explosion,
    Class_BloodHit,
    Class_ExplosionCloud,
    Class_ExplosionSmoke
  };

public:
  ParticleSystem();
  virtual ~ParticleSystem();

  /// Prepares starting or restarting the system
  void PrepareStart(void);
  /// Finishes the system
  virtual void Finish(void);

  /// Renders the particles and updates the system
  virtual void Render(float fDeltaTime) = 0;

  /// Returns true if finished and can be deleted or recycled
  inline bool IsFinished(void) { return m_bFinished; }
  /// Returns how long the system has been running
  inline float GetTime(void) { return m_fTime; }

protected:
  /// Time in seconds
  float m_fTime;

private:
  /// Is finished and can be deleted or recycled
  bool m_bFinished;
};
