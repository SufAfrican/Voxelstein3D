#include "StdAfx.h"
#include "Game/Entities/PlayerEntity.h"
#include "Game/Entities/DoorEntity.h"
#include "Game/Entities/ItemEntity.h"
#include "Game/Entities/UsableEntity.h"
#include "Game/Weapons/Weapon.h"
#include "Game/Weapons/Pistol.h"
#include "Game/Weapons/Knife.h"
#include "Game/Weapons/Chaingun.h"
#include "Game/Weapons/Grenade.h"
#include "Game/Weapons/StickyBomb.h"

// todo: remove this cheat code hack
#include <windows.h>


PlayerEntity::PlayerEntity()
{
  m_Classes.push_back(Class_Player);
  m_iLastJumpTick = 0;
  m_iSelectedWeapon = 0;
  m_Weapons.resize(Weapon_Num, NULL);
  m_CollectedAmmo.resize(Weapon_Num, 0);

  m_fCharacterRadius = Config::Player_fRadius;
  m_fCharacterHeight = Config::Player_fHeight;
  m_fCharacterStepHeight = Config::Player_fStepHeight;
  m_fEyeHeight = Config::Player_fHeight * 0.5f - 2;
  m_vCollisionBox = Vector3D(Config::Player_fRadius * 2, Config::Player_fHeight, Config::Player_fRadius * 2);
  m_vSize = m_vCollisionBox;
  m_CollisionType = Collision_Player;

  m_iHealth = 100;
  m_iLastTickDamage = 0;

  GiveWeaponItem(Weapon_Knife, true, -1);

  m_iLastFoundTreasure = 0;
  m_iLastFoundChaingun = 0;
  m_iHoldingTriggerTime = 0;

  m_bHasFlashlight = false;
}

PlayerEntity::~PlayerEntity()
{
  for(unsigned int i = 0; i < m_Weapons.size(); i++)
    delete m_Weapons[i];
}

Weapon *PlayerEntity::GetSelectedWeapon(void)
{
  if(m_iSelectedWeapon < m_Weapons.size())
    return m_Weapons[m_iSelectedWeapon];

  return NULL;
}

void PlayerEntity::GiveWeaponItem(WeaponNumber iWeapon, bool bGiveWeapon, int iGiveAmmo)
{
  // give weapon
  if(bGiveWeapon && m_Weapons[iWeapon] == NULL)
  {
    switch(iWeapon)
    {
      case Weapon_Knife: m_Weapons[iWeapon] = new Knife(this); break;
      case Weapon_Pistol: m_Weapons[iWeapon] = new Pistol(this); break;
      case Weapon_Chaingun: m_Weapons[iWeapon] = new Chaingun(this); break;
      case Weapon_Grenade: m_Weapons[iWeapon] = new Grenade(this); break;
      case Weapon_StickyBomb: m_Weapons[iWeapon] = new StickyBomb(this); break;
      default: return;
    };

    m_iSelectedWeapon = iWeapon;
    m_Weapons[iWeapon]->CreateFirstPersonSprites();
    m_Weapons[iWeapon]->SetAmmo(m_CollectedAmmo[iWeapon]);
    m_CollectedAmmo[iWeapon] = 0;
  }

  // give ammo
  if(m_Weapons[iWeapon])
    m_Weapons[iWeapon]->SetAmmo(m_Weapons[iWeapon]->GetAmmo() + iGiveAmmo);
  else
    m_CollectedAmmo[iWeapon] += iGiveAmmo;
}

void PlayerEntity::ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
{
  m_iLastTickDamage += Damage.iAmount;
  m_iHealth -= Damage.iAmount;
  // do the "death"
  if(m_iHealth <= 0)
  {
    m_vVelocity = Vector3D(0,0,0);
    m_iHealth = 0;
    m_fEyeHeight = 0;
    if(GetSelectedWeapon())
      GetSelectedWeapon()->SetSpriteVisibility(false);
    m_iSelectedWeapon = -1;
  }
}

void PlayerEntity::Tick(void)
{
  float fDeltaTime = GetGame()->GetTickDuration();

  // red screen effect from damage
  if(m_iLastTickDamage > 0)
  {
    float fDamageFactor = Clamp(m_iLastTickDamage, 0, 30) / 30.0f;
    int iColorFactor = Clamp(256 - (int)(fDamageFactor*255), 0, 256);
    Voxlap::SetColorFilter(256,iColorFactor,iColorFactor,fDamageFactor + 1.0f);
    m_iLastTickDamage = 0;
  }
  
  // todo: remove this cheat code hack
  static bool bCheatActive = false;
  // ILM MIL LIM
  if((GetKeyState('I') & 0xfe) && (GetKeyState('L') & 0xfe) && (GetKeyState('M') & 0xfe))
  {
    if(!bCheatActive)
    {
      Sound *pSound = GetSoundManager()->CreateSound2D("ogg/ken_is_angry.ogg");
      pSound->SetVolume(0.5f);
      pSound->Play();
      m_iHealth = 100;
      for(int i = PlayerEntity::Weapon_Pistol; i < PlayerEntity::Weapon_Num; i++)
      {
        GiveWeaponItem((PlayerEntity::WeaponNumber)i, true, 999);
      }
      GiveFlashlight();
      bCheatActive = true;
    }
  }
  else
  {
    bCheatActive = false;
  }


  // mouse rotation
  if(m_iHealth > 0)
  {
    m_vRotation.x += GetGame()->GetMouse().x * fDeltaTime;
    m_vRotation.y += GetGame()->GetMouse().y * fDeltaTime;
    m_vRotation.x = WrapRadians(m_vRotation.x);
    m_vRotation.y = Clamp(m_vRotation.y, -Math::fPi * 0.49f, Math::fPi * 0.49f);
  }

  Matrix44 mCamRot = MatrixFromEuler(m_vRotation);
  GetGame()->GetCameraMatrix().Set33(mCamRot);

  // player movement
  if(m_iHealth > 0)
  {
    // forward
    Vector3D vInput(0,0,0);
    if(GetGame()->IsKeyDown(Game::Key_Forward))
      vInput += mCamRot.GetFrontVector();
    // backward
    else if(GetGame()->IsKeyDown(Game::Key_Backward))
      vInput -= mCamRot.GetFrontVector();
    // left
    if(GetGame()->IsKeyDown(Game::Key_Left))
      vInput -= mCamRot.GetSideVector();
    // right
    else if(GetGame()->IsKeyDown(Game::Key_Right))
      vInput += mCamRot.GetSideVector();

    // clamp input speed
    vInput.y = 0;
    float fLength = vInput.Length();
    if(fLength > 1.0f)
      vInput *= 1.0f / fLength;

    // add velocities
    m_vVelocity.x = vInput.x * Config::Player_fMoveSpeed;
    m_vVelocity.z = vInput.z * Config::Player_fMoveSpeed;
    m_vVelocity.y -= Config::Player_fGravity * fDeltaTime; // gravity

    // jump
    if(GetGame()->IsKeyDown(Game::Key_Jump)
      && m_bIsOnFloor
      && GetGame()->GetCurrentTick() - m_iLastJumpTick > Config::Player_fJumpDelay * Game::fTicksPerSecond)
    {
      m_iLastJumpTick = GetGame()->GetCurrentTick();
      m_vVelocity.y += Config::Player_fJumpSpeed;
    }

    // crouch
    if(GetGame()->IsKeyDown(Game::Key_Crouch))
    {
      m_fCharacterHeight = Config::Player_fCrouchHeight;
      m_fEyeHeight = m_fCharacterHeight * 0.5f - 2;
      m_vCollisionBox = Vector3D(Config::Player_fRadius * 2, m_fCharacterHeight, Config::Player_fRadius * 2);
    }
    // stand back up
    else if(m_fCharacterHeight != Config::Player_fHeight)
    {
      // test that can stand up
      Vector3D vMoveUp = Vector3D(0,Config::Player_fHeight - m_fCharacterHeight,0);
      Vector3D vMove = CharacterClipMove(vMoveUp);
      if(vMove.y == vMoveUp.y)
      {
        m_fCharacterHeight = Config::Player_fHeight;
        m_fEyeHeight = m_fCharacterHeight * 0.5f - 2;
        m_vCollisionBox = Vector3D(Config::Player_fRadius * 2, m_fCharacterHeight, Config::Player_fRadius * 2);
      }
    }

    // update flashlight
    if(m_bHasFlashlight && GetGame()->IsKeyDown(Game::Key_Flashlight))
    {
      HitInfo info;
      Vector3D vSource = m_vPosition + Vector3D(0,m_fEyeHeight,0);
      Vector3D vSourceOrg = vSource;
      extern bool clipmove_slide; // hax!
      clipmove_slide = false;
      vSource = Voxlap::ClipMove(vSource, mCamRot.GetFrontVector() * Config::Player_fFlashLightDistance, 10);
      clipmove_slide = true;
      GetGame()->SetFlashLight(true, vSource, 1.0f);
    }
    else
    {
      GetGame()->SetFlashLight(false, Vector3D(), 0);
    }

    // pressed use
    if(GetGame()->IsKeyDown(Game::Key_Use))
    {
      HitInfo info;
      Vector3D vSource = m_vPosition + Vector3D(0,m_fEyeHeight,0);
      bool bHit = HitScan(vSource, mCamRot.GetFrontVector(), Config::Player_fUseDistance, info);
      Entity *pEntity = info.pEntity;
      if(bHit && pEntity != NULL) 
      {
        if(pEntity->GetClass() == Class_Door)
        {
          DoorEntity *pDoor = (DoorEntity *)pEntity;
          if(fabsf(m_vPosition.x - pDoor->m_vPosition.x) < Config::Door_fOpenDistance &&
            fabsf(m_vPosition.y - pDoor->m_vPosition.y) < Config::Door_fOpenDistance &&
            fabsf(m_vPosition.z - pDoor->m_vPosition.z) < Config::Door_fOpenDistance)
          pDoor->ToggleDoorOpen(m_vPosition);
        }
        else if(pEntity->IsOfClass(Class_Usable))
        {
          UsableEntity *pUsable = (UsableEntity *)pEntity;
          pUsable->PressedUse(this, vSource, info.vPos);
        }
      }
    }

    // pickup items
    {
      Vector3D vPickupPos = m_vPosition + Vector3D(0,m_fEyeHeight,0);
      std::vector<Entity *> Entities;
      GetGame()->GetEntities(m_vPosition - Vector3D(30,30,30), m_vPosition + Vector3D(30,30,30), Entities);
      for(unsigned int i = 0; i < Entities.size(); i++)
      {
        if(!Entities[i]->IsOfClass(Class_Item))
          continue;

        ItemEntity *pItem = (ItemEntity *)Entities[i];

        // something is in the way
        Vector3D vDir = pItem->m_vPosition - vPickupPos;
        float fDist = vDir.Normalize();
        Entity::HitInfo info;
        if(HitScan(vPickupPos, vDir, fDist, info) && 
          // sprite may be owned by another entity..
          (info.pEntity != pItem && (pItem->m_pSprite == NULL || info.pSprite != pItem->m_pSprite)))
          continue;

        pItem->OnPickup(this);
      }
    }

    // change selected weapon
    for(unsigned int i = 0; i < m_Weapons.size() && i < 9; i++)
    {
      if(GetGame()->IsKeyDown(Game::Key_Weapon1 + i) && m_Weapons[i] != NULL)
      {
        m_iSelectedWeapon = i;
      }
    }
  }

  // update character
  CharacterEntity::Tick();

  // update camera
  GetGame()->GetCameraMatrix().SetTranslation(m_vPosition + Vector3D(0,m_fEyeHeight,0));

  if(m_iHealth > 0)
  {
    // update weapons
    Weapon *pSelectedWeapon = GetSelectedWeapon();
    for(unsigned int i = 0; i < m_Weapons.size(); i++)
    {
      Weapon *pWeapon = m_Weapons[i];
      if(pWeapon == NULL)
        continue;

      // update trigger
      if(pWeapon == pSelectedWeapon)
      {
        pWeapon->SetSpriteVisibility(true);
        bool bTriggerDown = GetGame()->IsKeyDown(Game::Key_Fire);
        pWeapon->SetTriggerDown(bTriggerDown);
        if(bTriggerDown && !pWeapon->m_bFiresByClick)
        {
          m_iHoldingTriggerTime++;
        }
        else if(m_iHoldingTriggerTime > 0)
        {
          m_iHoldingTriggerTime = Min(m_iHoldingTriggerTime - 1, Game::iTicksPerSecond*2);
        }


        // smooth rotation
        Quaternion qLastRot = QuaternionFromMatrix(pWeapon->m_mSpriteToWorld);
        Quaternion qRot = QuaternionFromMatrix(GetGame()->GetCameraMatrix());     
        pWeapon->m_mSpriteToWorld = MatrixFromQuaternion(Slerp(qLastRot, qRot, 0.4f));
        pWeapon->m_mSpriteToWorld.SetTranslation(pWeapon->m_vWalkSwing + GetGame()->GetCameraMatrix().GetTranslation());

        // swing weapon
        if(m_bIsOnFloor && (fabsf(m_vVelocity.x) > 10.0f || fabsf(m_vVelocity.z) > 10.0f))
        {
          pWeapon->m_vWalkSwing.z += sinf(GetGame()->GetTime() * pWeapon->m_fSwingingSpeed) * pWeapon->m_fSwingingAmount;
          pWeapon->m_vWalkSwing.y += sinf(GetGame()->GetTime() * pWeapon->m_fSwingingSpeed) * pWeapon->m_fSwingingAmount;
        }
      }
      else
      {
        pWeapon->m_mSpriteToWorld = Matrix44::mIdentity;
        pWeapon->SetSpriteVisibility(false);
        pWeapon->SetTriggerDown(false);
      }

      pWeapon->m_vWalkSwing *= 0.9f;

      // tick
      pWeapon->SetLaunchParams(m_vPosition + Vector3D(0,m_fEyeHeight,0), mCamRot.GetFrontVector());
      pWeapon->Tick();
    }
  }

  // test level finish
  bool bLevelFinish = false;
  if(m_vPosition.x >= -9)
    bLevelFinish = true;
  if(m_vPosition.x >= -340 && (m_vPosition.z <= 9 || m_vPosition.z >= 1008))
    bLevelFinish = true;
  if(bLevelFinish)
  {
    GetGame()->SetLevelFinished();
  }
}

PlayerEntity::Stats::Stats() : iKills(0), iTreasures(0), iItems(0)
{
}
