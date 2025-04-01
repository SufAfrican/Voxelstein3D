#include "StdAfx.h"
#include "Game/Entities/DoorEntity.h"

// Create entity from placeholder
#include "Engine/SXLParser.h"
SXLPARSER_Start("kv6\\bluedoor.kv6")
{
  bKeepSprite = false;
  DoorEntity *pEntity = new DoorEntity("kv6\\bluedoor.kv6", mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();
SXLPARSER_Start("kv6\\irondr.kv6")
{
  bKeepSprite = false;
  DoorEntity *pEntity = new DoorEntity("kv6\\irondr.kv6", mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();
SXLPARSER_Start("kv6\\irondr2.kv6")
{
  bKeepSprite = false;
  DoorEntity *pEntity = new DoorEntity("kv6\\irondr2.kv6", mTransform);
  pEntity->m_bSlide = true;
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();

DoorEntity::DoorEntity(const char *strSpriteFile, const Matrix44 &mTrans)
{
  m_Classes.push_back(Class_Door);
  m_pSprite = new Sprite(strSpriteFile);
  m_pSprite->SetTransform(mTrans);
  m_CollisionSprites.push_back(m_pSprite);
  m_mOriginal = mTrans; 
  m_fCurrentRotation = 0;
  m_fTargetRotation = 0;
  m_bMoving = false;
  m_bSlide = false;

  UpdateCollision();
}

DoorEntity::~DoorEntity()
{
  delete m_pSprite;
}

void DoorEntity::ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
{
  if(Damage.Type == DamageType_HighExplosive)
  {
    GetGame()->RemoveEntity(this);
    return;
  }
  DamageSprite(m_pSprite, Damage);
}

void DoorEntity::ToggleDoorOpen(const Vector3D &vOpener)
{
  if(!m_bMoving)
  {
    if(m_fTargetRotation == 0)
    {
      // determine which direction should rotate to
      Vector3D vHingePos = m_pSprite->GetTransform().GetTranslation();
      Vector3D vDirToCenter = Normalize(m_vPosition - vOpener);
      Vector3D vDirToHinge = Normalize(vHingePos - vOpener);
      if(Cross(vDirToCenter, vDirToHinge).y > 0.0f)
        m_fTargetRotation = -Math::fPi * 0.5f;
      else
        m_fTargetRotation = Math::fPi * 0.5f;
    }
    else
    {
      m_fTargetRotation = 0;
    }

    Sound *pSound = GetSoundManager()->CreateSound("Wolf3D:622777,6629,0,6629");
    pSound->Play(m_vPosition, 1.0f);

    m_bMoving = true;
  }
}

void DoorEntity::UpdateCollision(void)
{
  Vector3D vMin, vMax;
  m_pSprite->CalculateBoundingBox(vMin, vMax, true, false);
  m_vCollisionBox = (vMax - vMin);
  SetSize(vMax - vMin);
  SetPosition((vMax + vMin) * 0.5f);
  m_CollisionType = Collision_Default;
}

void DoorEntity::Tick(void)
{
  if(m_bMoving)
  {
    float fDiff = Clamp(m_fTargetRotation - m_fCurrentRotation, -Config::Door_fRotationSpeed, Config::Door_fRotationSpeed);
    m_fCurrentRotation += fDiff;

    if(fabsf(fDiff) < 0.001f)
    {
      m_bMoving = false;
      return;
    }

    if(m_bSlide)
    {
      Matrix44 mDoorRot = Matrix44::mIdentity;
      mDoorRot.Set33(m_mOriginal);
      Matrix44 mDoorRotInv = Inverse(mDoorRot);

      Matrix44 mDoor = m_mOriginal;
      mDoor.SetTranslation( mDoor.GetTranslation() + Transform(mDoorRotInv,Vector3D(0,-fabsf(m_fCurrentRotation)*30,0)) );
      m_pSprite->SetTransform(mDoor);
    }
    else
    {
      Matrix44 mRotation = MatrixFromQuaternion(QuaternionFromAxisAngle(Vector3D(0,1,0), m_fCurrentRotation));

      Matrix44 mDoor = m_mOriginal;
      mDoor.SetTranslation(Vector3D(0,0,0));
      mDoor = Multiply(mDoor, mRotation);
      mDoor.SetTranslation(m_mOriginal.GetTranslation());
      m_pSprite->SetTransform(mDoor);
    }

    UpdateCollision();
  }
}

