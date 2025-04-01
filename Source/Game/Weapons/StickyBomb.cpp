#include "StdAfx.h"
#include "Game/Weapons/StickyBomb.h"
#include "Game/Entities/StickyBombEntity.h"

StickyBomb::StickyBomb(Entity *pShooter) : Weapon(pShooter)
{
  m_iFireDelay = Config::StickyBomb_fFireDelay * Game::fTicksPerSecond;
  m_bFiresByClick = Config::StickyBomb_bFiresByClick;
  m_iAmmo = 0;
}

StickyBomb::~StickyBomb()
{
}

void StickyBomb::CreateFirstPersonSprites(void)
{
  m_pHUDImage = Voxlap::LoadImage("png/hud_c4.png");

  // todo..
}

void StickyBomb::Animate(void)
{
  Weapon::Animate();
}

void StickyBomb::Fire(void)
{
  if(m_iAmmo <= 0)
    return;

  Entity::HitInfo info;
  if(HitScan(m_vLaunchPos, m_vLaunchDir, Config::StickyBomb_fPlacingDistance, info))
  {  
    GetGame()->AddEntity(new StickyBombEntity(m_pShooter, info.vPos, Voxlap::GetNormal(info.vPos)));    
    m_iAmmo--;
  }
}
