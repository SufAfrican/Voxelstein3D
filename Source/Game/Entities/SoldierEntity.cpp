#include "StdAfx.h"
#include "Game/Entities/SoldierEntity.h"
#include "Game/Entities/PlayerEntity.h"
#include "Game/ParticleSystems/ParticleSystemManager.h"
#include "Game/ParticleSystems/BloodHit.h"
#include "Game/ParticleSystems/BulletHit.h"
#include "Game/Weapons/Weapon.h"

SoldierEntity::SoldierEntity()
{
  m_Classes.push_back(Class_Soldier);

  m_vLookDir = Vector3D(0,0,1);
  m_vShootTarget = Vector3D(0,0,0);
  m_fViewDistance = 1000.0f;
  m_fViewCosAngle = -0.1f;
  m_CollisionType = Collision_Character;
  m_vMoveTarget = Vector3D(0,0,0);
  m_pWeapon = NULL;
  m_iHealth = 100;
  m_iShootDelay = 5;
  m_iShootPrepareDelay = 2;
  m_iMoveToNewPositionDelay = 20;
  m_iDamagedDelay = 25;
  m_vSpriteOffset = Vector3D(0,0,0);
  m_mSpriteRotation = Matrix44::mIdentity;
  m_vSpriteScale = Vector3D(1,1,1);
  m_iLastShoot = 0;
  m_fShootInaccuracy = 0;
  m_iLastPrepareShoot = 0;
  m_pTarget = NULL;
  m_bStateStackLocked = false;
  m_bHasRagdoll = false;

  m_iAnimGuard = m_iAnimWalk = m_iAnimPrepareShoot = m_iAnimShoot = m_iAnimHurt = m_iAnimDie = -1;

  GetGame()->GetStats().iEnemies++;
}

SoldierEntity::~SoldierEntity()
{
  ClearStates();
  delete m_pSprite;
  delete m_pWeapon;
}

bool SoldierEntity::CanSee(Entity *pEntity, const Vector3D &vPos)
{
  if(pEntity == NULL)
    return false;

  Vector3D vDirToPlayer = (pEntity->m_vPosition - (vPos + Vector3D(0,m_fEyeHeight,0)));
  float fDistToPlayer = vDirToPlayer.Normalize();

  // cone visibility check
  if(fDistToPlayer > 50.0f && Dot(vDirToPlayer, m_vLookDir) < m_fViewCosAngle)
    return false;

  // ray check
  Entity::HitInfo info;
  if(!HitScan(vPos + Vector3D(0,m_fEyeHeight,0), vDirToPlayer, Min(fDistToPlayer, m_fViewDistance), info)
    || info.pEntity != pEntity)
  {
    // didn't hit player
    return false;
  }
  return true;
}

PlayerEntity *SoldierEntity::GetNearestVisiblePlayer(void)
{
  Entity *pNearest = NULL;
  float fNearest = FLT_MAX;
  std::vector<Entity *> Players;
  // todo: fix this for network
  Players.push_back(GetGame()->GetLocalPlayer());
  for(unsigned int i = 0; i < Players.size(); i++)
  {
    Entity *pEntity = Players[i];
    if(!CanSee(pEntity, m_vPosition))
      continue;

    float fDist = (pEntity->m_vPosition - m_vPosition).SquaredLength();
    if(fDist < fNearest)
    {
      fNearest = fDist;
      pNearest = pEntity;
    }
  }
  return (PlayerEntity *)pNearest;
}

PlayerEntity *SoldierEntity::GetNearestPlayer(const Vector3D &vPos)
{
  Entity *pNearest = NULL;
  float fNearest = FLT_MAX;
  std::vector<Entity *> Players;
  // todo: fix this for network
  Players.push_back(GetGame()->GetLocalPlayer());
  for(unsigned int i = 0; i < Players.size(); i++)
  {
    Entity *pEntity = Players[i];
    float fDist = (pEntity->m_vPosition - vPos).SquaredLength();
    if(fDist < fNearest)
    {
      fNearest = fDist;
      pNearest = pEntity;
    }
  }
  return (PlayerEntity *)pNearest;
}

void SoldierEntity::LookAt(const Vector3D &vPos)
{
  Vector3D vDirToPlayer = vPos - m_vPosition;
  float fDistToPlayer = vDirToPlayer.Normalize();
  
  float fWantedAngle = GetEuler(vDirToPlayer).x;
  float fCurrentAngle = m_vRotation.x;
  Vector2D vWanted = Vector2D(sinf(fWantedAngle), -cosf(fWantedAngle));
  Vector2D vCurrent = Vector2D(sinf(fCurrentAngle), -cosf(fCurrentAngle));
  float fDiffAngle = Sign(Cross(vCurrent,vWanted)) * acosf(Clamp(Dot(vCurrent,vWanted), 0.0f, 1.0f));

  float fRotationSpeed = 0.1f;
  m_vRotation.x += Clamp(fDiffAngle, -fRotationSpeed, fRotationSpeed);
  m_vLookDir = Vector3D(vDirToPlayer.x, 0, vDirToPlayer.z);
  m_vLookDir.Normalize();
}

void SoldierEntity::ReceiveDamage(const DamageInfo &Damage, Entity *pDamager)
{
  m_LastDamage = Damage;

  bool bHitIndestructible = false;
  if(Damage.pHitSprite != NULL)
  {
    // test if hit indestructible sprite
    for(unsigned int i = 0; i < m_IndestructibleSprites.size(); i++)
    {
      if(m_IndestructibleSprites[i] != NULL && m_IndestructibleSprites[i]->GetSprite() == Damage.pHitSprite)
      {
        bHitIndestructible = true;
        break;
      }
    }

    if(bHitIndestructible)
    {
      GetParticles()->CreateSystem<BulletHitParticleSystem>()->Start(Damage.vHitPos, Damage.vHitNormal, 0);
    }
    else
    {
      // add wound
      if(Damage.iAmount > 15)
        Voxlap::DrawVoxelWound(Damage.pHitSprite, Damage.vHitPos, 3, 5);
      else
        Voxlap::DrawVoxelWound(Damage.pHitSprite, Damage.vHitPos, 1, 3);

      GetParticles()->CreateSystem<BloodHitParticleSystem>(15)->Start(Damage.vHitPos, Damage.vHitNormal);

      float fPitch = 0.8f + 0.4f * rand()/(float)RAND_MAX;
      Sound *pSound = GetSoundManager()->CreateSound("ogg/weapons/hit_flesh.ogg");
      pSound->SetRange(10.0f, 300.0f);
      pSound->Play(Damage.vHitPos, fPitch);
    }
  }

  // add ragdoll impulse
  if(m_pSprite->IsRagdoll())
  {
    AddRagdollImpulse(Damage);
  }

  int iDamageAmount = Damage.iAmount;
  if(bHitIndestructible)
    iDamageAmount = 0;

  // give damage
  if(m_iHealth > 0)
  {
    Vector3D vSoundPos = m_vPosition;

    m_iHealth -= iDamageAmount;
    if(m_iHealth > 0)
    {
      // attack if not already attacking
      if(m_pTarget == NULL && pDamager->IsOfClass(Entity::Class_Player))
      {
        m_pTarget = (PlayerEntity *)pDamager;
        GotoState(State_Attack);
      }
      if(GetCurrentState() != State_Damaged)
        PushState(State_Damaged);
    }
    else
    {
      m_iHealth = 0;

      ClearStates();
      GotoState(State_Dead);
      if(m_pWeapon)
        m_pWeapon->SetTriggerDown(false);
      m_vVelocity = Vector3D(0,0,0);
    }
  }
}

void SoldierEntity::OnHearSound(const Vector3D &vPos, Entity *pEntity)
{
  // turn toward the sound
  if(m_pTarget == NULL)
  {
    // if sound came from another soldier, copy his look direction
    if(pEntity->IsOfClass(Class_Soldier))
      m_vPeekDir = ((SoldierEntity *)pEntity)->m_vLookDir;
    else
      m_vPeekDir = vPos - m_vPosition;

    m_vPeekDir.y = 0;
    m_vPeekDir.Normalize();
    m_vPeekReturnDir = m_vLookDir;


    if(GetCurrentState() == State_Guard)
    {
      PushState(State_Peeking);
    }
    else if(GetCurrentState() == State_Peeking)
    {
      PopState();
      PushState(State_Peeking);
    }
  }
}

void SoldierEntity::AddRagdollImpulse(const DamageInfo &Damage)
{
  if(Damage.Type == DamageType_HighExplosive)
  {
    m_pSprite->AddRadialImpulse(Damage.vHitPos, Damage.iAmount * 0.01f, 20.0f);
    return;
  }

  float fFactor = 0.5f;
  if(Damage.Type == DamageType_Explosion)
    fFactor = 0.1f;
  m_pSprite->AddImpulse(Damage.vHitPos, Damage.vHitDir * Damage.iAmount * fFactor);
}


void SoldierEntity::Tick(void)
{
  float fDeltaTime = GetGame()->GetTickDuration();

  // update AI state
  if(!m_StateStack.empty())
  {
    TickState(m_StateStack.back());
  }

  bool bDead = GetCurrentState() == State_Dead;

  // apply gravity
  if(!bDead)
  {
    m_vVelocity.y -= 200 * fDeltaTime;
  }

  // update position
  CharacterEntity::Tick();

  // update sprite transform
  if(!bDead)
  {
    Matrix44 mTrans = MatrixFromEuler(m_vRotation);
    mTrans = Multiply(mTrans, m_mSpriteRotation);
    mTrans.Scale(m_vSpriteScale);
    mTrans.SetTranslation(m_vPosition + m_vSpriteOffset);
    m_pSprite->SetTransform(mTrans);
  }
  m_pSprite->Run(fDeltaTime);

  // update weapon
  if(m_pWeapon)
  {
    Vector3D vShootPos = m_vPosition + Vector3D(0,m_fEyeHeight,0);
    Vector3D vRandom(-0.5f + rand()/(float)RAND_MAX, -0.5f + rand()/(float)RAND_MAX, -0.5f + rand()/(float)RAND_MAX);
    Vector3D vShootDir = Normalize(Normalize(m_vShootTarget - vShootPos) + vRandom * m_fShootInaccuracy);
    m_pWeapon->SetLaunchParams(vShootPos, vShootDir);
    m_pWeapon->Tick();
  }

  //Voxlap::AddDebugLine(m_vPosition, m_vPosition - Vector3D(0,m_fCharacterHeight * 0.5f,0), 255,0,0);
  //Voxlap::AddDebugLine(m_vPosition, m_vPosition + 20.0f * mTrans.GetFrontVector(), 255,0,0);
}

bool SoldierEntity::MoveToAttackPosition(void)
{
  if(m_pTarget == NULL)
    return false;

  // look at target
  LookAt(m_pTarget->m_vPosition);

  float fStart = Time::GetTime();
  // move to random position closer to player
  Vector3D vNewPos = m_vPosition * 0.6f + m_pTarget->m_vPosition * 0.4f
    + 300 * Vector3D(-0.5f + rand()/(float)RAND_MAX, 0, -0.5f + rand()/(float)RAND_MAX);
  vNewPos = CharacterClipMove(vNewPos - m_vPosition) + m_vPosition;

  if(CanSee(m_pTarget, vNewPos))
  {
    //Voxlap::AddDebugLine(m_vPosition, vNewPos, 255,0,0, true);
    m_vMoveTarget = vNewPos;
    return true;
  }
  return false;
}

void SoldierEntity::ClearStates(void)
{
  // stack is locked
  if(m_bStateStackLocked)
  {
    m_StateStackOpQueue.push_back(StateStackOp(StateStackOp::Clear, 0));
    return;
  }

  while(!m_StateStack.empty())
  {
    EndState(m_StateStack.back());
    m_StateStack.pop_back();
  }
}

void SoldierEntity::PopState(void)
{
  // stack is locked
  if(m_bStateStackLocked)
  {
    m_StateStackOpQueue.push_back(StateStackOp(StateStackOp::Pop, 0));
    return;
  }

  if(!m_StateStack.empty())
  {
    EndState(m_StateStack.back());
    m_StateStack.pop_back();
  }
}

void SoldierEntity::PushState(int iStateType)
{
  // stack is locked
  if(m_bStateStackLocked)
  {
    m_StateStackOpQueue.push_back(StateStackOp(StateStackOp::Push, iStateType));
    return;
  }

  m_StateStack.push_back(State(iStateType));
}

void SoldierEntity::GotoState(int iStateType)
{
  // stack is locked
  if(m_bStateStackLocked)
  {
    m_StateStackOpQueue.push_back(StateStackOp(StateStackOp::Goto, iStateType));
    return;
  }

  if(!m_StateStack.empty())
  {
    EndState(m_StateStack.back());
    m_StateStack.pop_back();
  }
  m_StateStack.push_back(State(iStateType));
}

void SoldierEntity::TickState(State &state)
{
  m_bStateStackLocked = true;

  switch(state.iType)
  {
    case State_Guard: GuardState_Tick(state); break;
    case State_Move: MoveState_Tick(state); break;
    case State_Attack: AttackState_Tick(state); break;
    case State_Damaged: DamagedState_Tick(state); break;
    case State_Dead: DeadState_Tick(state); break;
    case State_Peeking: PeekingState_Tick(state); break;
  };
  state.iTime++;

  m_bStateStackLocked = false;
  ProcessStateStackOpQueue();
}

void SoldierEntity::EndState(State &state)
{
  // make a copy so we don't need to lock
  State stateCopy = state;
  switch(stateCopy.iType)
  {
    case State_Dead: DeadState_End(stateCopy); break;
  };
}

void SoldierEntity::ProcessStateStackOpQueue(void)
{
  if(m_bStateStackLocked)
    return;

  while(!m_StateStackOpQueue.empty())
  {
    StateStackOp &op = m_StateStackOpQueue.back();
    switch(op.Op)
    {
      case StateStackOp::Pop: PopState(); break;
      case StateStackOp::Push: PushState(op.iStateType); break;
      case StateStackOp::Goto: GotoState(op.iStateType); break;
      case StateStackOp::Clear: ClearStates(); break;
    }
    m_StateStackOpQueue.pop_back();
  }
}

void SoldierEntity::GuardState_Tick(State &state)
{
  // play guarding anim
  m_pSprite->PlayAnim(m_iAnimGuard, true);

  // look forward
  m_vLookDir = GetDirectionVector(m_vRotation);

  // try to find target
  PlayerEntity *pTarget = GetNearestVisiblePlayer();
  if(pTarget != NULL)
  {
    m_pTarget = pTarget;

    // start attack
    GotoState(State_Attack);
    return;
  }
}

void SoldierEntity::MoveState_Tick(State &state)
{
  bool bFinishedMove = false;

  // add velocity toward target
  Vector3D vDelta = m_vMoveTarget - m_vPosition;
  vDelta.y = 0;
  float fDeltaLen = vDelta.Length();
  if(fDeltaLen > 10.0f)
  {
    vDelta *= 1.0f / fDeltaLen;
    m_vVelocity.x = vDelta.x * 100;
    m_vVelocity.z = vDelta.z * 100;

    // play walking animation
    m_pSprite->PlayAnim(m_iAnimWalk, true);
  }
  else
  {
    m_vVelocity.x = 0;
    m_vVelocity.z = 0;
    bFinishedMove = true;
  }

  // always look where moving
  LookAt(m_vMoveTarget);

  // finished waiting or waited too long
  if(bFinishedMove || state.iTime > 3 * Game::iTicksPerSecond)
  {
    // stop walking animation
    m_pSprite->PlayAnim(m_iAnimGuard, false);
    // switch state
    PopState();
    return;
  }
}

void SoldierEntity::AttackState_Tick(State &state)
{
  if(state.iTime == 0)
  {
    // play alert
    if(!m_AlertSounds.empty())
    {
      GetSoundManager()->CreateSound(m_AlertSounds[rand()%m_AlertSounds.size()])->Play(m_vPosition, 1.0f);
      SendSoundEvent(m_vPosition, 300.0f);
    }

    // first move to attack position
    if(MoveToAttackPosition())
      PushState(State_Move);
    return;
  }

  LookAt(m_pTarget->m_vPosition);

  bool bCanSeeTarget = CanSee(m_pTarget, m_vPosition);

  if(m_pWeapon)
  {
    // enough time passed since last shoot
    if(bCanSeeTarget && GetGame()->GetCurrentTick() - m_iLastShoot > m_iShootDelay)
    {
      Vector3D vRotDir = GetDirectionVector(m_vRotation);
      float fCosAngleToTarget = Dot(vRotDir, m_vLookDir);
      if(fCosAngleToTarget > 0.8f)
      {
        // must first prepare to shoot
        if(m_pSprite->m_iCurrentAnim != m_iAnimPrepareShoot
          && m_pSprite->m_iCurrentAnim != m_iAnimShoot)
        {
          m_pSprite->PlayAnim(m_iAnimPrepareShoot, false);
          m_pSprite->m_fCurrentTime = 0.0f;
          m_iLastPrepareShoot = GetGame()->GetCurrentTick();
        }
        else
        {
          bool bFacingTarget = fCosAngleToTarget > 0.95f;
          bool bAlreadyShooting = m_pSprite->m_iCurrentAnim == m_iAnimShoot;
          bool bPrepareShootFinished = m_pSprite->m_iCurrentAnim == m_iAnimPrepareShoot
            && (GetGame()->GetCurrentTick() - m_iLastPrepareShoot > m_iShootPrepareDelay);
          if(bFacingTarget && (bAlreadyShooting || bPrepareShootFinished))
          {
            // shoot
            m_pSprite->PlayAnim(m_iAnimShoot, false);
            m_pSprite->m_fCurrentTime = 0.0f;
            m_vShootTarget = m_pTarget->m_vPosition + Vector3D(0,m_pTarget->m_fEyeHeight - 5.0f,0);
            m_pWeapon->SetTriggerDown(true);
            m_iLastShoot = GetGame()->GetCurrentTick();
          }
        }
      }
    }
    else
    {
      m_pWeapon->SetTriggerDown(false);
    }
  }

  if(!bCanSeeTarget || (state.iTime % m_iMoveToNewPositionDelay) == 0)
  {
    // move to new pos
    if(MoveToAttackPosition())
    {
      // stop shooting
      if(m_pWeapon)
        m_pWeapon->SetTriggerDown(false);
      PushState(State_Move);
    }
    return;
  }
}

void SoldierEntity::DamagedState_Tick(State &state)
{
  if(state.iTime == 0)
  {
    if(!m_DamageSounds.empty())
      GetSoundManager()->CreateSound(m_DamageSounds[rand()%m_DamageSounds.size()])->Play(m_vPosition, 1.0f);

    m_vVelocity = Vector3D(0,0,0);
    m_pSprite->PlayAnim(m_iAnimHurt, false);
  }

  // just wait
  if(state.iTime > m_iDamagedDelay)
  {
    PopState();
    return;
  }
}

void SoldierEntity::OnCreateRagdoll(void)
{
}

void SoldierEntity::DeadState_Tick(State &state)
{
  if(state.iTime == 0)
  {
    // play death sound
    if(!m_DeathSounds.empty())
    {
      GetSoundManager()->CreateSound(m_DeathSounds[rand()%m_DeathSounds.size()])->Play(m_vPosition, 1.0f);
      SendSoundEvent(m_vPosition, 300.0f);
    }

    // add kill
    GetGame()->GetLocalPlayer()->GetStats().iKills++;
    
    if(m_bHasRagdoll)
    {
      m_pSprite->CreateRagdoll( m_vSpriteScale.x );
      m_CollisionType = Collision_Ragdoll;
      OnCreateRagdoll();
      // add ragdoll impulse
      AddRagdollImpulse(m_LastDamage);
    }
    else
    {
      m_pSprite->PlayAnim(m_iAnimDie, false);
      m_CollisionType = Collision_Ragdoll;
    }

    // stop clipping
    m_bCharacterMoveClip = false;
  }

  if(m_bHasRagdoll || m_pSprite->m_fCurrentTime < 1.0f)
  {
    // update bounding box
    Vector3D vMin, vMax;
    m_pSprite->CalculateBoundingBox(vMin, vMax, true, false);
    SetSize(vMax - vMin);
    SetPosition((vMin+vMax) * 0.5f);
  }
}

void SoldierEntity::DeadState_End(State &state)
{
  m_pSprite->DestroyRagdoll();
  m_bCharacterMoveClip = true;
}

void SoldierEntity::PeekingState_Tick(State &state)
{
  // try to find target
  PlayerEntity *pTarget = GetNearestVisiblePlayer();
  if(pTarget != NULL)
  {
    m_pTarget = pTarget;

    // start attack
    GotoState(State_Attack);
    return;
  }

  if(state.iTime < 2 * Game::iTicksPerSecond)
  {
    LookAt(m_vPosition + m_vPeekDir);
  }
  else if(state.iTime > 4 * Game::iTicksPerSecond)
  {
    if(state.iTime < 6 * Game::iTicksPerSecond)
      LookAt(m_vPosition + m_vPeekReturnDir);
    else
      PopState();
  }
}