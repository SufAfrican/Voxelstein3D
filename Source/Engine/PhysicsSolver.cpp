#include "StdAfx.h"

#ifndef PROJECT_ANIMATOR
  #include "Game/Entities/Entity.h"
#endif

static const float fParticleSleepingThreshold = 0.05f;
PhysicsSolver *g_pPhysicsSolver = NULL;

PhysicsSolver::PhysicsSolver()
{
}

PhysicsSolver::~PhysicsSolver()
{
}

unsigned int PhysicsSolver::AddParticle(const Vector3D &vPosition, float fRadius)
{
  // find free one
  if(!m_FreeParticles.empty())
  {
    unsigned int iIndex = m_FreeParticles.back();
    m_FreeParticles.pop_back();
    m_Particles[iIndex] = Particle(vPosition, fRadius);
    // todo: sort to maintain cache efficiency
    m_ActiveParticles.push_back(iIndex);
    return iIndex;
  }
  // otherwise add new one
  m_Particles.push_back(Particle(vPosition, fRadius));
  m_ActiveParticles.push_back(m_Particles.size() - 1);
  return m_Particles.size() - 1;
}

void PhysicsSolver::RemoveParticle(unsigned int iParticle)
{
  for(unsigned int i = 0; i < m_ActiveParticles.size(); i++)
  {
    if(m_ActiveParticles[i] == iParticle)
    {
      m_ActiveParticles[i] = m_ActiveParticles.back();
      m_ActiveParticles.pop_back();
      m_FreeParticles.push_back(iParticle);
      return;
    }
  }
}

unsigned int PhysicsSolver::AddConstraint(unsigned int iParticle1, unsigned int iParticle2, float fLength)
{
  // compute length if not given
  if(fLength == -FLT_MAX)
  {
    Particle *pPart1 = GetParticle(iParticle1);
    Particle *pPart2 = GetParticle(iParticle2);
    if(!pPart1 || !pPart2)
      fLength = 0.0f;
    else
      fLength = (pPart1->vPos - pPart2->vPos).Length();
  }

  // find free one
  if(!m_FreeConstraints.empty())
  {
    unsigned int iIndex = m_FreeConstraints.back();
    m_FreeConstraints.pop_back();
    m_Constraints[iIndex] = Constraint(iParticle1, iParticle2, fLength);
    // todo: sort to maintain cache efficiency
    m_ActiveConstraints.push_back(iIndex);
    return iIndex;
  }

  // otherwise add new one
  m_Constraints.push_back(Constraint(iParticle1, iParticle2, fLength));
  m_ActiveConstraints.push_back(m_Constraints.size() - 1);
  return m_Constraints.size() - 1;
}

void PhysicsSolver::RemoveConstraint(unsigned int iConstraint)
{
  for(unsigned int i = 0; i < m_ActiveConstraints.size(); i++)
  {
    if(m_ActiveConstraints[i] == iConstraint)
    {
      m_ActiveConstraints[i] = m_ActiveConstraints.back();
      m_ActiveConstraints.pop_back();
      m_FreeConstraints.push_back(iConstraint);
      return;
    }
  }
}

void PhysicsSolver::AddStick(Stick &stick, const Vector3D &vPosition, const Vector3D &vDir, float fLength, float fRadius)
{
  stick.iParticle[0] = AddParticle(vPosition, fRadius);
  stick.iParticle[1] = AddParticle(vPosition + vDir * fLength * 0.5f, fRadius);
  stick.iParticle[2] = AddParticle(vPosition + vDir * fLength, fRadius);

  stick.iConstraint[0] = AddConstraint(stick.iParticle[0], stick.iParticle[1]);
  stick.iConstraint[1] = AddConstraint(stick.iParticle[0], stick.iParticle[2]);
  stick.iConstraint[2] = AddConstraint(stick.iParticle[1], stick.iParticle[2]);

  stick.qRot = Quaternion::qIdentity;
  stick.vFront = vDir;
}

void PhysicsSolver::GetTransform(Stick &stick, Matrix44 &m)
{
  Particle *pParticle[3];
  pParticle[0] = &m_Particles[stick.iParticle[0]];
  pParticle[1] = &m_Particles[stick.iParticle[1]];
  pParticle[2] = &m_Particles[stick.iParticle[2]];

  Vector3D vFront = Normalize(pParticle[2]->vPos - pParticle[0]->vPos);
  Vector3D vFrontOld = stick.vFront;
  stick.vFront = vFront;
  
  Vector3D vAxis = Normalize(Cross(vFront, vFrontOld));
  float fAngle = acosf(Clamp(Dot(vFront, vFrontOld), 0.0f, 1.0f));

  Quaternion qRot = QuaternionFromAxisAngle(vAxis, fAngle);
  stick.qRot *= qRot;
  stick.qRot.Normalize();

  m = MatrixFromQuaternion(stick.qRot);
  m.SetTranslation(pParticle[1]->vPos);
}

void PhysicsSolver::SetVelocity(Stick &stick, const Vector3D &vVelocity)
{
  for(int i = 0; i < 3; i++)
  {
    Particle *pParticle = &m_Particles[stick.iParticle[i]];
    pParticle->vOldPos = pParticle->vPos - vVelocity;
  }
}

void PhysicsSolver::RemoveStick(Stick &stick)
{
  for(int i = 0; i < 3; i++)
    RemoveParticle(stick.iParticle[i]);

  for(int i = 0; i < 3; i++)
    RemoveConstraint(stick.iConstraint[i]);
}

void PhysicsSolver::AddBox(Box &box, const Vector3D &vPosition, const Vector3D &vHalfSize)
{
  float fRadius = 3.0f;
  box.iParticle[0] = AddParticle(vPosition + Vector3D(-vHalfSize.x, -vHalfSize.y, -vHalfSize.z), fRadius);
  box.iParticle[1] = AddParticle(vPosition + Vector3D( vHalfSize.x, -vHalfSize.y, -vHalfSize.z), fRadius);
  box.iParticle[2] = AddParticle(vPosition + Vector3D( vHalfSize.x,  vHalfSize.y, -vHalfSize.z), fRadius);
  box.iParticle[3] = AddParticle(vPosition + Vector3D(-vHalfSize.x,  vHalfSize.y, -vHalfSize.z), fRadius);

  box.iParticle[4] = AddParticle(vPosition + Vector3D(-vHalfSize.x, -vHalfSize.y, vHalfSize.z), fRadius);
  box.iParticle[5] = AddParticle(vPosition + Vector3D( vHalfSize.x, -vHalfSize.y, vHalfSize.z), fRadius);
  box.iParticle[6] = AddParticle(vPosition + Vector3D( vHalfSize.x,  vHalfSize.y, vHalfSize.z), fRadius);
  box.iParticle[7] = AddParticle(vPosition + Vector3D(-vHalfSize.x,  vHalfSize.y, vHalfSize.z), fRadius);

  float fSmallest = vHalfSize.x;
  if(vHalfSize.y < fSmallest)
    fSmallest = vHalfSize.y;
  if(vHalfSize.z < fSmallest)
    fSmallest = vHalfSize.z;
  box.iParticle[8] = AddParticle(vPosition, fSmallest);

  int c = 0;
  for(int i = 0; i < 9; i++)
  {
    for(int j = i + 1; j < 9; j++)
    {
      box.iConstraint[c] = AddConstraint(box.iParticle[i], box.iParticle[j]);
      c++;
    }
  }
}

void PhysicsSolver::GetTransform(Box &box, Matrix44 &m)
{
  Particle *pParticle[9];

  for(int i = 0; i < 9; i++)
  {
    pParticle[i] = &m_Particles[box.iParticle[i]];
  }

  Vector3D vX = Normalize(pParticle[1]->vPos - pParticle[0]->vPos);
  Vector3D vY = Normalize(pParticle[3]->vPos - pParticle[0]->vPos);
  Vector3D vZ = Normalize(pParticle[4]->vPos - pParticle[0]->vPos);

  m._11 = vX.x;
  m._12 = vX.y;
  m._13 = vX.z;
  m._14 = 0;

  m._21 = vY.x;
  m._22 = vY.y;
  m._23 = vY.z;
  m._24 = 0;

  m._31 = vZ.x;
  m._32 = vZ.y;
  m._33 = vZ.z;
  m._34 = 0;

  m._41 = pParticle[8]->vPos.x;
  m._42 = pParticle[8]->vPos.y;
  m._43 = pParticle[8]->vPos.z;
  m._44 = 1;
}

void PhysicsSolver::SetVelocity(Box &box, const Vector3D &vVelocity)
{
  for(int i = 0; i < 9; i++)
  {
    Particle *pParticle = &m_Particles[box.iParticle[i]];
    pParticle->vOldPos = pParticle->vPos - vVelocity;
  }
}

void PhysicsSolver::RemoveBox(Box &box)
{
  for(int i = 0; i < 9; i++)
    RemoveParticle(box.iParticle[i]);

  for(int i = 0; i < 36; i++)
    RemoveConstraint(box.iConstraint[i]);
}

inline void ApplyFriction(Vector3D &vOldPos, Vector3D &vPos, const Vector3D &vFriction)
{
  vOldPos.x = LerpFloat(vOldPos.x, vPos.x, vFriction.x);
  vOldPos.y = LerpFloat(vOldPos.y, vPos.y, vFriction.y);
  vOldPos.z = LerpFloat(vOldPos.z, vPos.z, vFriction.z);
}

inline Vector3D PhysicsSolver::GetClippedMove(Particle &particle, const Vector3D &vMove)
{
#ifdef PROJECT_ANIMATOR
  if(particle.fRadius > 0.0f && particle.vPos.y + vMove.y - particle.fRadius < 0.0f)
    return Vector3D(vMove.x, particle.fRadius - particle.vPos.y, vMove.z);
  return vMove;
#else
  if(particle.fRadius > 0.0f)
  {
    unsigned int iCollisionMask = ~(Entity::Collision_Player); // don't collide with player..
    Vector3D vEntityClipped = GetGame()->ClipMove(particle.vPos, Vector3D(particle.fRadius, particle.fRadius, particle.fRadius)*2, vMove, iCollisionMask) - particle.vPos;
    return Voxlap::ClipMove(particle.vPos, vEntityClipped, particle.fRadius) - particle.vPos;
  }
  return vMove;
#endif
}

#ifdef PROJECT_ANIMATOR

#else

#define PARTICLE_CLIPPEDMOVE(move) \
    if(particle.fRadius > 0.0f) \
      vClippedMove = ;

#endif


void PhysicsSolver::Tick(void)
{
  Vector3D vGravity = Vector3D(0,-0.08f,0);

  // iterate particles
  for(unsigned int i = 0; i < m_ActiveParticles.size(); i++)
  {
    Particle &particle = m_Particles[m_ActiveParticles[i]];

    Vector3D vSpeed = particle.vPos - particle.vOldPos;

    if(particle.IsSleeping())
    {
      if(vSpeed.SquaredLength() < fParticleSleepingThreshold)
        continue;
      particle.iSleepCounter = 0;
    }

    Vector3D vMove = vSpeed + vGravity;

    // test for collision
    Vector3D vClippedMove = GetClippedMove(particle, vMove);

    // increment position
    Vector3D vTemp = particle.vPos;
    particle.vPos += vClippedMove;
    particle.vOldPos = vTemp;

    float fFloorHit = vClippedMove.y - vMove.y;
    if(fFloorHit > 0.05f)
    {
      // fake friction
      float fFriction = Clamp(1.0f - 0.05f / fFloorHit, 0.0f, 0.5f);
      particle.vFriction = Vector3D(fFriction,0,fFriction);
      ApplyFriction(particle.vOldPos, particle.vPos, particle.vFriction);
    }
    else
    {
      particle.vFriction = Vector3D(0,0,0);
    }

    /// sleeping threshold
    float fSpeed = (particle.vPos - particle.vOldPos).SquaredLength();
    if(fSpeed < fParticleSleepingThreshold)
    {
      particle.iSleepCounter++;
    }
    else
    {
      particle.iSleepCounter = 0;
    }
  }

#ifndef PROJECT_ANIMATOR
  if(Config::Debug_bVisualizePhysicsParticles)
  {
    for(unsigned int i = 0; i < m_ActiveParticles.size(); i++)
    {
      Particle &particle = m_Particles[m_ActiveParticles[i]];
      Vector3D vHalfSize(particle.fRadius, particle.fRadius, particle.fRadius);
      Voxlap::AddDebugBoxLines(particle.vPos - Vector3D(1,1,1), particle.vPos + Vector3D(1,1,1), 255,0,0);
      Voxlap::AddDebugBoxLines(particle.vPos - vHalfSize, particle.vPos + vHalfSize, 0,0,100);
    }
  }
#endif

  // iterate constraints
  for(int it = 0; it < 5; it++)
  {
    for(unsigned int i = 0; i < m_ActiveConstraints.size(); i++)
    {
      Constraint &constraint = m_Constraints[m_ActiveConstraints[i]];
      Particle &particle1 = m_Particles[constraint.iP1];
      Particle &particle2 = m_Particles[constraint.iP2];

      if(particle1.IsSleeping() && particle2.IsSleeping())
        continue;

      Vector3D vDelta = (particle2.vPos + particle2.vConstraintMove) - (particle1.vPos + particle1.vConstraintMove);
      float fDeltaLength = vDelta.Length();

      // inequality constraint (only min distance enforced)
      float fLength = constraint.fLength;
      if(fLength < 0.0f)
      {
        fLength = -fLength;
        if(fDeltaLength > fLength)
          continue;
      }

      float fError = (fDeltaLength - fLength);
      vDelta *= 0.5f * (fError) / fDeltaLength;
      particle1.vConstraintMove += vDelta;
      particle2.vConstraintMove -= vDelta;
      //Voxlap::AddDebugLine(particle1.vPos, particle2.vPos, 0, 255, 0);
    }
    // make sure constraints don't invalidate position
    for(unsigned int i = 0; i < m_ActiveParticles.size(); i++)
    {
      Particle &particle = m_Particles[m_ActiveParticles[i]];
      if(particle.vConstraintMove.SquaredLength() > 0.001f)
      {
        Vector3D vClippedMove = GetClippedMove(particle, particle.vConstraintMove);

        // didn't move upward
        if(vClippedMove.y <= 0.0f)
        {
          // apply friction to move
          ApplyFriction(vClippedMove, Vector3D(0,0,0), particle.vFriction);
        }

        // wake up if necessary
        if(particle.IsSleeping() && vClippedMove.SquaredLength() >= fParticleSleepingThreshold)
          particle.WakeUp();

        particle.vPos += vClippedMove;
        particle.vConstraintMove = Vector3D(0,0,0);
      }
    }
  }
}
