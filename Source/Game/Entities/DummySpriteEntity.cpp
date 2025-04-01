#include "StdAfx.h"
#include "Game/Entities/DummySpriteEntity.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
SXLPARSER_Start("kv6\\safe.kv6")
{
  bKeepSprite = false;
  DummySpriteEntity *pEntity = new DummySpriteEntity("kv6\\safe.kv6", mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();

DummySpriteEntity::DummySpriteEntity(const char *strFile, const Matrix44 &mTransform)
{
  m_Classes.push_back(Class_DummySprite);
  m_pSprite = new Sprite(strFile);
  m_pSprite->SetTransform(mTransform);

  Vector3D vMin,vMax;
  m_pSprite->CalculateBoundingBox(vMin, vMax);
  m_vCollisionBox = m_vSize = vMax-vMin;
  m_vPosition = (vMax+vMin)*0.5f;
  m_CollisionSprites.push_back(m_pSprite);
  m_CollisionType = Entity::Collision_Default;
}

DummySpriteEntity::~DummySpriteEntity()
{
  delete m_pSprite;
}

void DummySpriteEntity::ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
{
  DamageSprite(m_pSprite, Damage);
}