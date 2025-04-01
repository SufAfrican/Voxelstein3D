#include "StdAfx.h"
#include "Game/Entities/CharacterEntity.h"

CharacterEntity::CharacterEntity()
{
  m_Classes.push_back(Class_Character);
  m_fCharacterRadius = 16.0f;
  m_fCharacterHeight = 50.0f;
  m_fCharacterStepHeight = 10.0f;
  m_bIsOnFloor = false;
  m_fEyeHeight = m_fCharacterHeight * 0.5f;
  m_bCharacterMoveClip = true;
}

CharacterEntity::~CharacterEntity()
{
}

void CharacterEntity::Tick(void)
{
  float fDeltaTime = GetGame()->GetTickDuration();
  float fFloor = Voxlap::GetFloorHeight(m_vPosition) + m_fCharacterHeight * 0.5f;

  // calculate new position
  Vector3D vNewPos = m_vPosition + m_vVelocity * fDeltaTime;

  if(!m_bCharacterMoveClip)
  {
    // move directly to position
    SetPosition(vNewPos);
  }
  else
  {
    // clamp position above floor
    if(vNewPos.y < fFloor)
    {
      vNewPos.y = fFloor;
      m_vVelocity.y = 0;
      m_bIsOnFloor = true;
    }
    else if(vNewPos.y > fFloor + 0.01f)
    {
      m_bIsOnFloor = false;
    }

    // finally move & collide
    Vector3D vMove = vNewPos - m_vPosition;
    Vector3D vClippedMove = CharacterClipMove(vMove);
    if(vMove.x != 0)
      m_vVelocity.x *= fabsf(vClippedMove.x/vMove.x);
    if(vMove.y != 0)
      m_vVelocity.y *= fabsf(vClippedMove.y/vMove.y);
    if(vMove.z != 0)
      m_vVelocity.z *= fabsf(vClippedMove.z/vMove.z);
    SetPosition(m_vPosition + vClippedMove);
  }
}

Vector3D CharacterEntity::CharacterClipMove(const Vector3D &vMove)
{
  Vector3D vSmallestMove(vMove);

  // do 3 move tests against voxels from top to bottom
  //
  Vector3D vTopPosition = m_vPosition + Vector3D(0, m_fCharacterHeight * 0.5f, 0);
  Vector3D vTestOffset(0, -(m_fCharacterHeight - m_fCharacterStepHeight - m_fCharacterRadius)/3.0f, 0);
  if(vTestOffset.y > 0)
    vTestOffset.y = 0;
  for(int i = 1; i <= 3; i++)
  {
    Vector3D vNewMoveDelta = Voxlap::ClipMove(vTopPosition + vTestOffset*i, vMove, m_fCharacterRadius) - vTopPosition - vTestOffset*i;

    if(fabsf(vNewMoveDelta.x) < fabsf(vSmallestMove.x))
      vSmallestMove.x = vNewMoveDelta.x;

    if(fabsf(vNewMoveDelta.y) < fabsf(vSmallestMove.y))
      vSmallestMove.y = vNewMoveDelta.y;

    if(fabsf(vNewMoveDelta.z) < fabsf(vSmallestMove.z))
      vSmallestMove.z = vNewMoveDelta.z;
  }

  // clip movement against entities
  unsigned int iCollisionTypeMask = (~Collision_Ragdoll); // don't collide with ragdolls
  vSmallestMove = ClipMove(m_vPosition, Vector3D(m_fCharacterRadius*2, m_fCharacterHeight, m_fCharacterRadius*2), vSmallestMove, iCollisionTypeMask) - m_vPosition;
  return vSmallestMove;
}