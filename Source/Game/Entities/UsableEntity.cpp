#include "StdAfx.h"
#include "Game/Entities/UsableEntity.h"
#include "Engine/SXLParser.h"

UsableEntity::UsableEntity()
{
  m_Classes.push_back(Class_Usable);
}

////////////////////////////////////////////
// RADIO
class UsableRadioEntity : public UsableEntity
{
public:
  Sprite *m_pSprite;
  SmartPointer<Sound> m_pSound;

  UsableRadioEntity(const char *strFile, const Matrix44 &mTransform)
  {
    m_pSprite = new Sprite(strFile);
    m_pSprite->SetTransform(mTransform);

    Vector3D vMin,vMax;
    m_pSprite->CalculateBoundingBox(vMin, vMax);
    m_vCollisionBox = m_vSize = vMax-vMin;
    m_vPosition = (vMax+vMin)*0.5f;
    m_CollisionSprites.push_back(m_pSprite);
    m_CollisionType = Entity::Collision_Default;
    m_iHealth = 1;
  }

  ~UsableRadioEntity()
  {
    delete m_pSprite;
  }

  void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
  {
    m_iHealth = 0;
    if(m_pSound != NULL)
    {
      m_pSound->Stop();
      m_pSound = NULL;
    }
    DamageSprite(m_pSprite, Damage);
  }
  
  void PressedUse(Entity *pEntity, const Vector3D &vSource, const Vector3D &vHit)
  {
    if(m_iHealth <= 0)
      return;

    if(m_pSound == NULL)
    {
      m_pSound = GetSoundManager()->CreateSound("ogg\\radiotalk.ogg");
    }
    if(!m_pSound->IsPlaying())
    {
      m_pSound->SetRange(20.0f, 400.0f);
      m_pSound->Play(m_vPosition, 1.0f);
    }
  }
};

SXLPARSER_Start("kv6\\radio.kv6")
{
  bKeepSprite = false;

  UsableRadioEntity *pEntity = new UsableRadioEntity("kv6\\radio.kv6", mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();
//
////////////////////////////////////////////


////////////////////////////////////////////
// SAFE
class UsableSafeEntity : public UsableEntity
{
public:
  Sprite *m_pSprite;
  SmartPointer<Sound> m_pSound;
  VoxlapImage *m_pSafeNumbers;
  Matrix44 m_mOriginal, m_mTransInv;
  int m_iNumber[4];
  unsigned int m_iLastChangeTick;
  bool m_bOpen, m_bOpening;
  SmartPointer<Sound> m_pSafeClick;

  UsableSafeEntity(const char *strFile, const Matrix44 &mTransform)
  {
    m_pSprite = new Sprite(strFile);
    m_pSprite->SetTransform(mTransform);
    m_mOriginal = mTransform;
    m_mTransInv = Inverse(mTransform);


    Vector3D vMin,vMax;
    m_pSprite->CalculateBoundingBox(vMin, vMax);
    m_vCollisionBox = m_vSize = vMax-vMin;
    m_vPosition = (vMax+vMin)*0.5f;
    m_CollisionSprites.push_back(m_pSprite);
    m_CollisionType = Entity::Collision_Default;
    m_iHealth = 1;

    m_pSafeNumbers = Voxlap::LoadImage("png/safenumbers.png");

    m_iNumber[0] = 9;
    m_iNumber[1] = 8;
    m_iNumber[2] = 6;
    m_iNumber[3] = 5;

    m_iLastChangeTick = 0;
    m_bOpen = false;
    m_bOpening = false;
    m_vRotation.x = 0;

    DrawNumbers();
  }

  ~UsableSafeEntity()
  {
    delete m_pSprite;
  }

  void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
  {
    DamageSprite(m_pSprite, Damage);
    DrawNumbers();
  }

  void PressedUse(Entity *pEntity, const Vector3D &vSource, const Vector3D &vHit)
  {
    if(GetGame()->GetCurrentTick() - m_iLastChangeTick <= Game::iTicksPerSecond/8)
      return;
    m_iLastChangeTick = GetGame()->GetCurrentTick();

    // toggle number
    Vector3D &vRel = Transform(m_mTransInv, vHit);
    for(int i = 0; i < 4; i++)
    {
      if(vRel.x >= 66 + i*9 && vRel.x <= 73 + i*9
        && vRel.y <= -2 && vRel.y >= -3
        && vRel.z <= -17 && vRel.z >= -33)
      {
        m_iNumber[i] = (m_iNumber[i] + 1) % 10;
        if(m_pSafeClick == NULL)
          m_pSafeClick = GetSoundManager()->CreateSound("ogg/safeclick.ogg");
        m_pSafeClick->Play(m_vPosition, 1.0f);
        break;
      }
    }

    // check numbers
    if(!m_bOpen && (m_iNumber[0] == 1 && m_iNumber[1] == 3 && m_iNumber[2] == 3 && m_iNumber[3] == 7))
    {
      GetSoundManager()->CreateSound("ogg/safeopen.ogg")->Play(m_vPosition, 1.0f);
      // open door
      m_bOpening = true;
      m_bOpen = true;
    }

    DrawNumbers();
  }

  void Tick(void)
  {
    if(m_bOpening)
    {
      m_vRotation.x -= 0.02f;
      if(m_vRotation.x < -Math::fPi * 0.5f)
      {
        m_vRotation.x = -Math::fPi * 0.5f;
        m_bOpening = false;
      }

      Matrix44 mRotation = MatrixFromQuaternion(QuaternionFromAxisAngle(Vector3D(0,1,0), m_vRotation.x));

      Matrix44 mDoor = m_mOriginal;
      mDoor.SetTranslation(Vector3D(0,0,30));
      mDoor = Multiply(mDoor, mRotation);
      mDoor.SetTranslation(mDoor.GetTranslation() - Vector3D(0,0,30) + m_mOriginal.GetTranslation());
      m_pSprite->SetTransform(mDoor);
      m_mTransInv = Inverse(mDoor);

      Vector3D vMin,vMax;
      m_pSprite->CalculateBoundingBox(vMin, vMax);
      m_vCollisionBox = vMax-vMin;
      SetPosition((vMax+vMin)*0.5f);
      SetSize(m_vCollisionBox);
    }
  }

  void DrawNumbers(void)
  {
#pragma warning(disable:4312)
    long *pPixels = (long *)m_pSafeNumbers->pData;
#pragma warning(default:4312)

    // draw numbers
    for(int i = 0; i < 4; i++)
    {
      for(int x = 0; x < 8; x++)
      {
        for(int y = 0; y < 15; y++)
        {
          int iColor = pPixels[x + (8 * m_iNumber[i])  + y * 80];
          Voxlap::SetVoxelColor(m_pSprite, x + 7 + (i*9), 10, y + 30, (iColor>>16)&0xFF,(iColor>>8)&0xFF,(iColor)&0xFF);
        }
      }
    }
  }
};

SXLPARSER_Start("kv6\\safedoor.kv6")
{
  bKeepSprite = false;

  UsableSafeEntity *pEntity = new UsableSafeEntity("kv6\\safedoor.kv6", mTransform);
  GetGame()->AddEntity(pEntity);
}
SXLPARSER_End();
//
////////////////////////////////////////////
