#include "StdAfx.h"
#include "Game/Entities/TreasureItemEntity.h"
#include "Game/Entities/PlayerEntity.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
////////////////////////////////////////////
SXLPARSER_Start("kv6\\trsr2.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(new TreasureItemEntity("kv6\\trsr2.kv6", mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\trsr4.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(new TreasureItemEntity("kv6\\trsr4.kv6", mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\goldbar.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(new TreasureItemEntity("kv6\\goldbar.kv6", mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\goldbar2.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(new TreasureItemEntity("kv6\\goldbar2.kv6", mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\goldbar3.kv6")
{
  bKeepSprite = false;
  GetGame()->AddEntity(new TreasureItemEntity("kv6\\goldbar3.kv6", mTransform));
}
SXLPARSER_End();
////////////////////////////////////////////
SXLPARSER_Start("kv6\\flashlight.kv6")
{
  bKeepSprite = false;
  TreasureItemEntity *pEntity = new TreasureItemEntity("kv6\\flashlight.kv6", mTransform);
  pEntity->m_bGiveFlashlight = true;
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();
////////////////////////////////////////////

TreasureItemEntity::TreasureItemEntity(const char *strSpriteFile, const Matrix44 & mTransform) : ItemEntity(strSpriteFile, mTransform)
{
  m_strPickupSound = "ogg/treasurepickup.ogg";
  m_bGiveFlashlight = false;

  GetGame()->GetStats().iTreasures++;
}

void TreasureItemEntity::OnPickup(PlayerEntity *pPlayer)
{
  pPlayer->m_iLastFoundTreasure = GetGame()->GetCurrentTick();
  pPlayer->GetStats().iTreasures++;

  if(m_strPickupSound != NULL)
  {
    Sound *pSound = GetSoundManager()->CreateSound2D(m_strPickupSound);
    pSound->SetVolume(0.3f);
    pSound->Play();
  }

  if(m_bGiveFlashlight)
    pPlayer->GiveFlashlight();

  GetGame()->RemoveEntity(this);
}
