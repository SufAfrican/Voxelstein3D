#include "StdAfx.h"
#include "Game/Weapons/Weapon.h"
#include "Game/Entities/Entity.h"
#include "Game/Entities/SoldierEntity.h"
#include "Game/ParticleSystems/ParticleSystem.h"


Weapon::Weapon(Entity *pShooter)
{
  m_iLastFire = 0;
  
  m_iFireDelay = 1.0f * Game::fTicksPerSecond;
  m_iLastFire = GetGame()->GetCurrentTick() + Game::iTicksPerSecond/4; // always wait after giving weapon
  m_bFiresByClick = false;

  m_bTriggerDown = false;
  m_bTriggerDownPreviously = false;
  m_vLaunchPos = Vector3D(0,0,0);
  m_vLaunchDir = Vector3D(0,0,1);
  m_mSpriteToWorld = Matrix44::mIdentity;

  m_pWeaponSprite = NULL;
  m_iMuzzleSpriteIndex = -1;
  m_iAnimIdle = -1;
  m_iAnimShoot = -1;
  m_pShooter = pShooter;
  m_vWalkSwing = Vector3D(0,0,0);

  m_iAmmo = 0;
  m_pHUDImage = NULL;
}

Weapon::~Weapon()
{
  delete m_pWeaponSprite;
}

void Weapon::SetTriggerDown(bool bDown)
{
  m_bTriggerDown = bDown;
}

void Weapon::SetSpriteVisibility(bool bVisible)
{
  if(m_pWeaponSprite)
  {
    for(unsigned int i = 0; i < m_pWeaponSprite->m_Sprites.size(); i++)
    {
      if(i != m_iMuzzleSpriteIndex)
      {
        m_pWeaponSprite->m_Sprites[i]->GetSprite()->SetVisible(bVisible);
      }
    }
  }
}

bool Weapon::CanFire(void)
{
  // needs to release button first
  if(m_bFiresByClick && m_bTriggerDownPreviously)
    return false;

  // not enough time passed
  if((int)(GetGame()->GetCurrentTick() - m_iLastFire) < (int)m_iFireDelay)
    return false;

  return true;
}

void Weapon::Animate(void)
{
  float fDeltaTime = GetGame()->GetTickDuration();
  if(m_pWeaponSprite != NULL)
  {
    // playing shoot
    if(m_pWeaponSprite->m_iCurrentAnim == m_iAnimShoot)
    {
      // finished animation
      if(!m_pWeaponSprite->m_bLooping && m_pWeaponSprite->m_fCurrentTime >= 1.0f)
      {
        // play idle
        m_pWeaponSprite->m_iCurrentAnim = m_iAnimIdle;
        m_pWeaponSprite->m_bLooping = true;
      }
    }

    m_pWeaponSprite->SetTransform(m_mSpriteToWorld);
    m_pWeaponSprite->Run(fDeltaTime);
  }
}

void Weapon::Tick(void)
{
  Animate();

  // holding down trigger
  if(m_bTriggerDown)
  {
    // test if can fire
    if(CanFire())
    {
      // fire
      Fire();
      m_iLastFire = GetGame()->GetCurrentTick();
    }
  }
  m_bTriggerDownPreviously = m_bTriggerDown;

  // update firing sounds
  for(unsigned int i = 0; i < m_FiringSounds.size();)
  {
    m_FiringSounds[i]->SetPosition(m_vLaunchPos);

    if(!m_FiringSounds[i]->IsPlaying())
    {
      m_FiringSounds[i] = m_FiringSounds.back();
      m_FiringSounds.pop_back();
      continue;
    }
    else
    {
       i++;
    }
  }
}

void Weapon::PlayRicochetSound(const Vector3D &vPos)
{
  float fPitch = 0.8f + 0.4f * rand()/(float)RAND_MAX;
  Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/hit.ogg");
  pSound->SetRange(10.0f, 200.0f);
  pSound->Play(vPos, fPitch);
}
