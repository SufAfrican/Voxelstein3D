#include "StdAfx.h"
#include "Game/Entities/ItemEntity.h"

ItemEntity::ItemEntity()
{
  m_Classes.push_back(Class_Item);
  m_pSprite = NULL;
  m_bOwnedSprite = false;
}

ItemEntity::ItemEntity(const char *strSpriteFile, const Matrix44 &mTransform)
{
  m_Classes.push_back(Class_Item);
  m_pSprite = NULL;
  m_bOwnedSprite = false;

  if(strSpriteFile != NULL)
  {
    m_pSprite = new Sprite(strSpriteFile);
    m_pSprite->SetTransform(mTransform);
    
    Vector3D vMin, vMax;
    m_pSprite->CalculateBoundingBox(vMin, vMax, true, false);
    m_vSize = (vMax - vMin);
    m_vPosition = (vMax + vMin) * 0.5f;

    m_bOwnedSprite = true;
  }
}

ItemEntity::~ItemEntity()
{
  if(m_pSprite)
  {
    if(m_bOwnedSprite)
      delete m_pSprite;
    else
      m_pSprite->SetVisible(false);
  }
}

void ItemEntity::Tick(void)
{
  if(!m_bOwnedSprite && m_pSprite != NULL)
  {
    Vector3D vMin, vMax;
    m_pSprite->CalculateBoundingBox(vMin, vMax, true, false);
    m_vSize = (vMax - vMin);
    m_vPosition = (vMin+vMax) * 0.5f;
  }
}