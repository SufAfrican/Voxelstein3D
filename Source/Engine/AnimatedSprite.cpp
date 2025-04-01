#include "StdAfx.h"

SpritePoser::SpritePoser(Sprite *pSprite)
{
  m_pSprite = pSprite;
  m_vPosition = Vector3D(0,0,0);
  m_qRotation = Quaternion::qIdentity;
  m_vScale = Vector3D(1,1,1);
}

SpritePoser::~SpritePoser()
{
  delete m_pSprite;
}

void SpritePoser::UpdateSprite(const Matrix44 &mTransform)
{
  Matrix44 mSprite = MatrixFromQuaternion(m_qRotation);
  mSprite.SetTranslation(m_vPosition);
  mSprite.Scale(m_vScale);
  mSprite = Multiply(mSprite, mTransform);
  m_pSprite->SetTransform(mSprite);
}

////////////////////////////////////////////////////////////////////////////////////////

AnimatedSprite::AnimatedSprite(AnimationFile *pFile)
{
  m_pFile = pFile;
  m_Sprites.resize(m_pFile->m_SpriteFiles.size());
  for(unsigned int i = 0; i < m_pFile->m_SpriteFiles.size(); i++)
  {
    m_Sprites[i] = new SpritePoser(new Sprite(m_pFile->m_SpriteFiles[i]));
  }
  m_fCurrentTime = 0;
  m_iCurrentAnim = -1;
  m_bLooping = true;
  m_bPlaying = false;
  m_mTransform = Matrix44::mIdentity;

  m_bRagdoll = false;
}

AnimatedSprite::~AnimatedSprite()
{
  DestroyRagdoll();
  for(unsigned int i = 0; i < m_Sprites.size(); i++)
    delete m_Sprites[i];
}

void AnimatedSprite::Run(float fDeltaTime, bool bApplyAnimation)
{
  if(m_bRagdoll)
  {
    UpdateFromRagdoll();
    return;
  }

  // has valid animation selected
  if(bApplyAnimation && (unsigned int)m_iCurrentAnim < m_pFile->m_Animations.size())
  {
    AnimationFile::Animation &anim = m_pFile->m_Animations[m_iCurrentAnim];

    if(m_bPlaying)
    {
      if(m_bLooping)
        m_fCurrentTime = fmodf(m_fCurrentTime + fDeltaTime / anim.fDuration, 1.0f);
      else
        m_fCurrentTime = Min(m_fCurrentTime + fDeltaTime / anim.fDuration, 1.0f);
    }

    // find frame
    unsigned int iNextFrame;
    for(iNextFrame = 0; iNextFrame < anim.Frames.size(); iNextFrame++)
    {
      if(m_fCurrentTime <= anim.Frames[iNextFrame].fTime)
        break;
    }


    // get last frame
    unsigned int iFrame = iNextFrame - 1;
    if(iFrame >= anim.Frames.size())
    {
      if(!m_bLooping)
        iFrame = 0;
      else
        iFrame = anim.Frames.size() - 1;
    }

    // use last available
    if(iNextFrame >= anim.Frames.size())
    {
      if(!m_bLooping)
        iNextFrame = anim.Frames.size() - 1;
      else
        iNextFrame = 0;
    }

    // safety check
    if(iFrame < anim.Frames.size() && iNextFrame < anim.Frames.size())
    {
      // apply states
      float fLerpFactor = 1.0f;
      if(iNextFrame > iFrame)
        fLerpFactor = (m_fCurrentTime - anim.Frames[iFrame].fTime) / (anim.Frames[iNextFrame].fTime - anim.Frames[iFrame].fTime);
      else
      {
        float fTimeBetweenLastAndFirstFrame = (1.0f - anim.Frames[iFrame].fTime) + anim.Frames[iNextFrame].fTime;
        if(iNextFrame == 0 && iFrame > 0 && fTimeBetweenLastAndFirstFrame > 0)
        {
          // is between last frame and end
          if(m_fCurrentTime > anim.Frames[iFrame].fTime)
          {
            fLerpFactor = (m_fCurrentTime - anim.Frames[iFrame].fTime) / fTimeBetweenLastAndFirstFrame;
          }
          // is between beginning and first frame
          else
          {
            float fTimeBetweenLastAndEnd = (1.0f - anim.Frames[iFrame].fTime);
            fLerpFactor = (m_fCurrentTime + fTimeBetweenLastAndEnd) / fTimeBetweenLastAndFirstFrame;
          }
        }
      }
      std::vector<AnimationFile::SpritePose> &Poses = anim.Frames[iFrame].Poses;
      std::vector<AnimationFile::SpritePose> &NextPoses = anim.Frames[iNextFrame].Poses;
      for(unsigned int i = 0; i < m_Sprites.size(); i++)
      {
        m_Sprites[i]->m_vScale = LerpVector3D(Poses[i].vScale, NextPoses[i].vScale, fLerpFactor);
        m_Sprites[i]->m_vPosition = LerpVector3D(Poses[i].vPosition, NextPoses[i].vPosition, fLerpFactor);
        m_Sprites[i]->m_qRotation = Slerp(Poses[i].qRotation, NextPoses[i].qRotation, fLerpFactor);
      }
    }
  }

  for(unsigned int i = 0; i < m_Sprites.size(); i++)
    m_Sprites[i]->UpdateSprite(m_mTransform);
}

void AnimatedSprite::CalculateBoundingBox(Vector3D &vMin, Vector3D &vMax, bool bTransformed, bool bAccurate)
{
  vMin = Vector3D(FLT_MAX,FLT_MAX,FLT_MAX);
  vMax = Vector3D(-FLT_MAX,-FLT_MAX,-FLT_MAX);
  for(unsigned int i = 0; i < m_Sprites.size(); i++)
  {
    Vector3D vMinSpr, vMaxSpr;
    if(!bTransformed)
    {
      m_Sprites[i]->UpdateSprite(Matrix44::mIdentity);
    }
    m_Sprites[i]->GetSprite()->CalculateBoundingBox(vMinSpr, vMaxSpr, bTransformed, bAccurate);
    for(int j = 0; j < 3; j++)
    {
      vMin[j] = Min(vMinSpr[j], vMin[j]);
      vMax[j] = Max(vMaxSpr[j], vMax[j]);
    }
  }
}

SpritePoser *AnimatedSprite::FindSprite(const char *strSprite)
{
  std::string strFilename(strSprite);
  MakeLowerCase(strFilename);
  for(unsigned int i = 0; i < m_Sprites.size(); i++)
  {
    if(m_Sprites[i]->GetSprite()->GetFilename() == strFilename)
      return m_Sprites[i];
  }
  return NULL;
}

void AnimatedSprite::UpdateFromRagdoll(void)
{
  UpdateRagdollTransforms();

  // don't allow falling asleep unless all particles can do it at once
  bool bAllParticlesCanSleep = true;
  for(unsigned int i = 0; i < m_Particles.size(); i++)
  {
    if(GetPhysics()->GetParticle(m_Particles[i])->iSleepCounter == 0)
    {
      bAllParticlesCanSleep = false;
      break;
    }
  }
  if(!bAllParticlesCanSleep)
  {
    for(unsigned int i = 0; i < m_Particles.size(); i++)
      GetPhysics()->GetParticle(m_Particles[i])->WakeUp();
  }
  
  // update sprite transforms
  for(unsigned int i = 0; i < m_RagdollSprites.size(); i++)
  {
    RagdollSprite &rag = m_RagdollSprites[i];

    Matrix44 mNewSprite = Multiply(m_mTransform, Multiply(rag.mInitialInverse, rag.mTransform));
    m_Sprites[rag.iSprite]->UpdateSprite(mNewSprite);
  }
}

void AnimatedSprite::UpdateRagdollTransforms(void)
{
  for(unsigned int i = 0; i < m_RagdollSprites.size(); i++)
  {
    RagdollSprite &rag = m_RagdollSprites[i];

    // only two particles
    if(rag.MovePointParticles.size() == 2)
    {
      // solve spherical rotation from direction
      //
      PhysicsSolver::Particle *pParticle[2] = 
      {
        GetPhysics()->GetParticle(rag.MovePointParticles[0]),
        GetPhysics()->GetParticle(rag.MovePointParticles[1])
      };

      Vector3D vCenter = (pParticle[1]->vPos + pParticle[0]->vPos) * 0.5f;
      Vector3D vDir = Normalize(pParticle[1]->vPos - pParticle[0]->vPos);
      Vector3D vX, vY, vZ;

      Vector3D vFront = Normalize(pParticle[1]->vPos - pParticle[0]->vPos);
      Vector3D vFrontOld = rag.vFront;
      rag.vFront = vFront;
      
      Vector3D vAxis = Normalize(Cross(vFront, vFrontOld));
      float fAngle = acosf(Clamp(Dot(vFront, vFrontOld), 0.0f, 1.0f));

      Quaternion qRot = QuaternionFromAxisAngle(vAxis, fAngle);
      rag.qRot *= qRot;
      rag.qRot.Normalize();

      rag.mTransform = MatrixFromQuaternion(rag.qRot);
      rag.mTransform.SetTranslation(pParticle[0]->vPos);
    }
    else if(rag.MovePointParticles.size() == 3)
    {
      // solve full rotation from plane
      //
      PhysicsSolver::Particle *pParticle[3] = 
      {
        GetPhysics()->GetParticle(rag.MovePointParticles[0]),
        GetPhysics()->GetParticle(rag.MovePointParticles[1]),
        GetPhysics()->GetParticle(rag.MovePointParticles[2])
      };

      Vector3D vCenter = (pParticle[0]->vPos + pParticle[1]->vPos + pParticle[2]->vPos) / 3.0f;

      Vector3D vLeft = Normalize(pParticle[1]->vPos - pParticle[0]->vPos);
      Vector3D vFront = Normalize(pParticle[2]->vPos - pParticle[0]->vPos);

      Vector3D vY = Normalize(Cross(vFront,vLeft));
      Vector3D vZ = Normalize(Cross(vY,-vLeft));
      Vector3D vX = Normalize(-Cross(vY,vZ));

      rag.mTransform._11 = vX.x;
      rag.mTransform._12 = vX.y;
      rag.mTransform._13 = vX.z;
      rag.mTransform._14 = 0;

      rag.mTransform._21 = vY.x;
      rag.mTransform._22 = vY.y;
      rag.mTransform._23 = vY.z;
      rag.mTransform._24 = 0;

      rag.mTransform._31 = vZ.x;
      rag.mTransform._32 = vZ.y;
      rag.mTransform._33 = vZ.z;
      rag.mTransform._34 = 0;

      rag.mTransform._41 = vCenter.x;
      rag.mTransform._42 = vCenter.y;
      rag.mTransform._43 = vCenter.z;
      rag.mTransform._44 = 1;
    }
    else if(rag.MovePointParticles.size() == 1)
    {
      rag.mTransform = Matrix44::mIdentity;
      rag.mTransform.SetTranslation(GetPhysics()->GetParticle(rag.MovePointParticles[0])->vPos);
    }
    else
    {
      rag.mTransform = Matrix44::mIdentity;
    }
  }
}

unsigned int AnimatedSprite::GetParticleIndex(unsigned int iSpritePhysics, unsigned int iParticle)
{
  if(iSpritePhysics >= m_pFile->m_SpritePhysics.size())
    return -1;
  if(iParticle >= m_pFile->m_SpritePhysics[iSpritePhysics].Particles.size())
    return -1;

  // get particle array offset
  unsigned int iParticleOffset = 0;
  for(unsigned int i = 0; i < iSpritePhysics; i++)
    iParticleOffset += m_pFile->m_SpritePhysics[i].Particles.size();

  return iParticleOffset + iParticle;
}

void AnimatedSprite::CreateRagdoll(float fScale)
{
  if(m_bRagdoll)
    DestroyRagdoll();

  // add particles
  for(unsigned int i = 0; i < m_pFile->m_SpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[i];

    unsigned int iParticleOffset = m_Particles.size();
    m_Particles.resize(iParticleOffset + phys.Particles.size());

    for(unsigned int j = 0; j < phys.Particles.size(); j++)
    {
      AnimationFile::PhysicsParticle &particle = phys.Particles[j];
      Vector3D vPos = Transform(m_Sprites[phys.iSprite]->GetSprite()->GetTransform(), particle.vPosition);
      m_Particles[iParticleOffset + j] = GetPhysics()->AddParticle(vPos, particle.fRadius * fScale);
    }
  }

  // add ragdoll settings
  m_RagdollSprites.resize(m_pFile->m_SpritePhysics.size());
  for(unsigned int i = 0; i < m_pFile->m_SpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[i];
    m_RagdollSprites[i].iSprite = phys.iSprite;

    unsigned int iParticle0 = GetParticleIndex(phys.MovePointParticles[0].iSpritePhysics, phys.MovePointParticles[0].iParticle);
    unsigned int iParticle1 = GetParticleIndex(phys.MovePointParticles[1].iSpritePhysics, phys.MovePointParticles[1].iParticle);
    unsigned int iParticle2 = GetParticleIndex(phys.MovePointParticles[2].iSpritePhysics, phys.MovePointParticles[2].iParticle);

    m_RagdollSprites[i].MovePointParticles.reserve(3);
    if(iParticle0 != -1)
      m_RagdollSprites[i].MovePointParticles.push_back(m_Particles[iParticle0]);
    if(iParticle1 != -1)
      m_RagdollSprites[i].MovePointParticles.push_back(m_Particles[iParticle1]);
    if(iParticle2 != -1)
      m_RagdollSprites[i].MovePointParticles.push_back(m_Particles[iParticle2]);

    if(m_RagdollSprites[i].MovePointParticles.size() < 1)
    {
      printf("WARNING: CreateRagdoll requires at least 1 movepoint particle to be set for each sprite!\n");
    }
    else if(m_RagdollSprites[i].MovePointParticles.size() >= 2)
    {
      Vector3D vPos0 = GetPhysics()->GetParticle(m_RagdollSprites[i].MovePointParticles[0])->vPos;
      Vector3D vPos1 = GetPhysics()->GetParticle(m_RagdollSprites[i].MovePointParticles[1])->vPos;
      m_RagdollSprites[i].vFront = Normalize(vPos1 - vPos0);
      m_RagdollSprites[i].qRot = Quaternion::qIdentity;
    }
  }

  // calculate default transform
  UpdateRagdollTransforms();
  for(unsigned int i = 0; i < m_RagdollSprites.size(); i++)
  {
    m_RagdollSprites[i].mInitialInverse = InverseFast(m_RagdollSprites[i].mTransform);
  }

  // add constraints
  for(unsigned int i = 0; i < m_pFile->m_PhysicsConstraints.size(); i++)
  {
    AnimationFile::PhysicsConstraint &constraint = m_pFile->m_PhysicsConstraints[i];

    // find particle index in m_Particles
    unsigned int iParticleArrayOffset1 = 0;
    for(unsigned int j = 0; j < constraint.iSpritePhysics1; j++)
      iParticleArrayOffset1 += m_pFile->m_SpritePhysics[j].Particles.size();

    // find particle index in m_Particles
    unsigned int iParticleArrayOffset2 = 0;
    for(unsigned int j = 0; j < constraint.iSpritePhysics2; j++)
      iParticleArrayOffset2 += m_pFile->m_SpritePhysics[j].Particles.size();

    unsigned int iParticle1 = m_Particles[iParticleArrayOffset1 + constraint.iParticle1];
    unsigned int iParticle2 = m_Particles[iParticleArrayOffset2 + constraint.iParticle2];
    if(constraint.fDistance != 0.0f)
      m_Constraints.push_back(GetPhysics()->AddConstraint(iParticle1, iParticle2, constraint.fDistance * fScale));
    else
      m_Constraints.push_back(GetPhysics()->AddConstraint(iParticle1, iParticle2));
    
  }

  m_bRagdoll = true;
}

void AnimatedSprite::DestroyRagdoll(void)
{
  if(!m_bRagdoll)
    return;
  
  for(unsigned int i = 0; i < m_Particles.size(); i++)
    GetPhysics()->RemoveParticle(m_Particles[i]);
  m_Particles.clear();
  for(unsigned int i = 0; i < m_Constraints.size(); i++)
    GetPhysics()->RemoveConstraint(m_Constraints[i]);
  m_Constraints.clear();

  m_RagdollSprites.clear();

  m_bRagdoll = false;
}

void AnimatedSprite::AddImpulse(const Vector3D &vPos, const Vector3D &vImpulse)
{
  if(!m_bRagdoll)
    return;

  float fNearestDist = FLT_MAX;
  PhysicsSolver::Particle *pNearest = NULL;

  // add full impulse to nearest
  for(unsigned int i = 0; i < m_Particles.size(); i++)
  {
    PhysicsSolver::Particle *pParticle = GetPhysics()->GetParticle(m_Particles[i]);
    pParticle->WakeUp();

    float fDist = (pParticle->vPos - vPos).SquaredLength();
    if(fDist < fNearestDist)
    {
      fNearestDist = fDist;
      pNearest = pParticle;
    }
  }
  if(pNearest)
  {
    pNearest->vOldPos -= vImpulse;    
  }
}

void AnimatedSprite::AddRadialImpulse(const Vector3D &vPos, float fImpulse, float fRadius)
{
  if(!m_bRagdoll)
    return;

  // just apply to all particles scaled by distance
  for(unsigned int i = 0; i < m_Particles.size(); i++)
  {
    PhysicsSolver::Particle *pParticle = GetPhysics()->GetParticle(m_Particles[i]);
    pParticle->WakeUp();

    Vector3D vDir = pParticle->vPos - vPos;
    float fDist = vDir.Normalize();

    if(fDist < fRadius)
      pParticle->vOldPos -= vDir * fImpulse;
    else
      pParticle->vOldPos -= vDir * fImpulse * (fRadius / fDist);
  }
}

