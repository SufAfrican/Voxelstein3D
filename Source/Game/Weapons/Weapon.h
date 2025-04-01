#pragma once

class Entity;
class ParticleSystem;
struct VoxlapImage;

/// Base class for weapons
class Weapon
{
public:
  Weapon(Entity *pShooter);
  virtual ~Weapon();

  /// Starts/stops firing
  virtual void SetTriggerDown(bool bDown);
  /// Sets projectile launch position and direction
  inline void SetLaunchParams(const Vector3D &vPos, const Vector3D &vDir) { m_vLaunchPos = vPos; m_vLaunchDir = vDir; }
  /// Sets ammo amount
  inline void SetAmmo(int iAmmo) { m_iAmmo = iAmmo; }
  /// Returns current amount of ammo
  inline int GetAmmo(void) { return m_iAmmo; }

  /// Creates sprites for player view
  virtual void CreateFirstPersonSprites(void) {}
  /// Sets weapon sprite visibility
  virtual void SetSpriteVisibility(bool bVisible);
  /// Returns HUD image
  inline VoxlapImage *GetHUDImage(void) { return m_pHUDImage; }

  /// Update weapon state
  virtual void Tick(void);

  /// Returns true holding trigger down
  inline bool IsTriggerDown(void) { return m_bTriggerDown; }

protected:
  /// Returns true if can fire the weapon
  virtual bool CanFire(void);
  /// Animates the weapon
  virtual void Animate(void);
  /// Fires the weapon
  virtual void Fire(void) = 0;
  /// Plays default ricochet sound
  void PlayRicochetSound(const Vector3D &vPos);

public:
  /// Weapon firing delay in ticks
  unsigned int m_iFireDelay;
  /// Does the weapon require constant clicking?
  bool m_bFiresByClick;

  /// Player holding down the trigger
  bool m_bTriggerDown;
  /// Player was holding down the trigger in last tick
  bool m_bTriggerDownPreviously;
  /// Tick where weapon was last fired
  unsigned int m_iLastFire;
  /// Position where projectile is fired from
  Vector3D m_vLaunchPos;
  /// Direction where projectile is launched to
  Vector3D m_vLaunchDir;

  /// Weapon sprite
  AnimatedSprite *m_pWeaponSprite;
  /// Muzzle locator sprite index
  int m_iMuzzleSpriteIndex;
  /// Idle animation index
  int m_iAnimIdle;
  /// Shoot animation index
  int m_iAnimShoot;
  /// HUD image
  VoxlapImage *m_pHUDImage;
  /// Shooting sounds
  std::vector< SmartPointer<Sound> > m_FiringSounds;

  /// Current amount of ammo
  int m_iAmmo;

  /// World space transformation for first person sprites
  Matrix44 m_mSpriteToWorld;
  /// Sprite walk-swing motion frequency
  float m_fSwingingSpeed;
  /// Sprite walk-swing motion amplitude
  float m_fSwingingAmount;
  /// Current walk-swing amount
  Vector3D m_vWalkSwing;

protected:
  /// Shooter entity
  Entity *m_pShooter;
};
