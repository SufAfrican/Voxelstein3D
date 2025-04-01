#pragma once

#include "Game/Entities/CharacterEntity.h"

class Weapon;
class PlayerEntity;

/// Base class for soldier enemies
class SoldierEntity : public CharacterEntity
{
public:
  SoldierEntity();
  ~SoldierEntity();

  /// Entity was damaged
  virtual void ReceiveDamage(const DamageInfo &Damage, Entity *pDamager);
  /// Shooting, dieing, alert etc. sound was made nearby
  virtual void OnHearSound(const Vector3D &vPos, Entity *pEntity);

  /// Updates the entity state
  void Tick(void);

public:
  /// Adds impulse to ragdoll based on damage
  void AddRagdollImpulse(const DamageInfo &Damage);

public:
  /// Tests if entity is visible from given position
  bool CanSee(Entity *pEntity, const Vector3D &vPos);
  /// Returns nearest visible player
  PlayerEntity *GetNearestVisiblePlayer(void);
  /// Returns nearest player from given position
  PlayerEntity *GetNearestPlayer(const Vector3D &vPos);

  /// Looks at given position
  void LookAt(const Vector3D &vPos);
  /// Moves to a position where can attack target
  bool MoveToAttackPosition(void);

  /// Called when creating ragdoll
  virtual void OnCreateRagdoll(void);

public:
  /// Possible states
  enum StateType
  {
    State_Invalid = 0,
    State_Guard = 1,
    State_Move = 2,
    State_Attack = 3,
    State_Damaged = 4,
    State_Dead = 5,
    State_Peeking = 6
  };

  /// Instance of a state
  struct State
  {
    State(int iType) : iType(iType), iTime(0) {}
    int iType;
    unsigned int iTime;
  };

protected:
  /// State stack
  std::vector<State> m_StateStack;

  /// Pending state operation
  struct StateStackOp
  {
    enum OperationType
    {
      Pop,
      Push,
      Goto,
      Clear
    };

    StateStackOp(OperationType Op, int iStateType) : Op(Op), iStateType(iStateType) {}
    OperationType Op;
    int iStateType;
  };
  /// Pending state operations
  std::vector<StateStackOp> m_StateStackOpQueue;
  /// Is state stack locked (operations will be queued)
  bool m_bStateStackLocked;
public:

  /// Removes all states
  void ClearStates(void);
  /// Removes current state
  void PopState(void);
  /// Pushes a state on top of stack
  void PushState(int iStateType);
  /// Replaces current state with given one
  void GotoState(int iStateType);
  /// Processes state stack operation queue
  void ProcessStateStackOpQueue(void);
  /// Returns current state type
  inline int GetCurrentState(void) { if(m_StateStack.empty()) return State_Invalid; return m_StateStack.back().iType; }

  /// Ticks a state
  virtual void TickState(State &state);
  /// Ends a state (state is removed from stack)
  virtual void EndState(State &state);

  void GuardState_Tick(State &state);
  void MoveState_Tick(State &state);
  void AttackState_Tick(State &state);
  void DamagedState_Tick(State &state);
  void DeadState_Tick(State &state);
  void DeadState_End(State &state);
  void PeekingState_Tick(State &state);

public:

  /// Target entity
  SmartPointer<PlayerEntity> m_pTarget;

  /// Point where should move
  Vector3D m_vMoveTarget;

  /// Direction where should peek at
  Vector3D m_vPeekDir;
  /// Direction where peeking should end
  Vector3D m_vPeekReturnDir;

  /// Current look direction
  Vector3D m_vLookDir;
  /// Current shooting direction
  Vector3D m_vShootTarget;
  /// Shooting inaccuracy
  float m_fShootInaccuracy;
  /// Tick where last shot
  unsigned int m_iLastShoot;
  /// Tick where last prepared to shoot
  unsigned int m_iLastPrepareShoot;

  /// Cosine of vision angle (for line of sight)
  float m_fViewCosAngle;
  /// How far enemy can see
  float m_fViewDistance;

  /// Sprite
  AnimatedSprite *m_pSprite;
  /// Sprite scale
  Vector3D m_vSpriteScale;
  /// Sprite rotation
  Matrix44 m_mSpriteRotation;
  /// Sprite offset
  Vector3D m_vSpriteOffset;
  /// Animations
  int m_iAnimGuard, m_iAnimWalk, m_iAnimPrepareShoot, m_iAnimShoot, m_iAnimHurt, m_iAnimDie;
  /// Has ragdoll
  bool m_bHasRagdoll;
  /// Sprites that receive no damage
  std::vector<SpritePoser *> m_IndestructibleSprites;

  /// Damage sounds
  std::vector<const char *> m_DamageSounds;
  /// Death sounds
  std::vector<const char *> m_DeathSounds;
  /// Alert sounds
  std::vector<const char *> m_AlertSounds;

  /// Weapon
  Weapon *m_pWeapon;

  /// Time between shots
  unsigned int m_iShootDelay;
  /// Time spent waiting before shooting
  unsigned int m_iShootPrepareDelay;
  /// Time before moving to new position
  unsigned int m_iMoveToNewPositionDelay;
  /// Time to recover after being damaged
  unsigned int m_iDamagedDelay;

  /// Last damage received before dying
  DamageInfo m_LastDamage;
};