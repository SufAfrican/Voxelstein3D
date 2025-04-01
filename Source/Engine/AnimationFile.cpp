#include "StdAfx.h"

#ifdef PROJECT_ANIMATOR

#include <commdlg.h>
#include <direct.h>
void PickNewSpriteFile(char *strFile)
{
  char strMessage[1024];
  sprintf(strMessage, "The file %s cannot be found. Pick a replacement?", strFile);

  int iRes = MessageBox(NULL, strMessage, "Animator", MB_ICONWARNING|MB_YESNOCANCEL);
  if(iRes != IDYES)
  {
    return;
  }

  char strNewFile[MAX_PATH];
  strcpy(strNewFile, strFile);

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = strNewFile;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFilter = TEXT("KV6 sprite (.kv6)\0*.kv6\0");
  ofn.lpstrDefExt = TEXT(".kv6");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  extern HWND ghwnd;
  ofn.hwndOwner = ghwnd;
  char strDir[MAX_PATH];
  GetModuleFileName(GetModuleHandle(NULL),strDir,MAX_PATH);
  ofn.lpstrInitialDir = strDir;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  bool bResult = GetOpenFileName(&ofn) ? true : false;

  // reset dir back
  std::string strPath = GetPath(strDir) + "\\";
  _chdir(strPath.c_str());

  if(bResult)
  {
    // make path relative
    if(_strnicmp(strNewFile, strPath.c_str(), strPath.size()) != 0)
    {
      MessageBox(NULL, "File is not under exe path!\n", "Error!", MB_OK);
      return;
    }
    strcpy(strFile, strNewFile + strPath.size());
  }
}
#endif

bool AnimationFile::LoadFrom(const char *strFile)
{
  FILE *pFile = fopen(strFile, "rb");
  if(pFile == NULL)
  {
    return false;
  }

  // read ID
  char strID[4] = {0};
  fread(strID, 1, 4, pFile);
  if(strID[0] != 'A' || strID[1] != 'N' || strID[2] != 'I' || strID[3] != 'M')
  {
    printf("ANIM identifier not found\n");
    return false;
  }

  // read version
  unsigned int iVersion = 0;
  fread(&iVersion, 1, 4, pFile);
  if(iVersion < 1 || iVersion > 2)
  {
    printf("Unsupported anim file version %i\n", iVersion);
    return false;
  }

  // read number of sprites
  unsigned int iNumSprites = 0;
  fread(&iNumSprites, 1, 4, pFile);

  // read all sprites
  m_SpriteFiles.resize(iNumSprites);
  for(unsigned int i = 0; i < iNumSprites; i++)
  {
    unsigned char iStringSize = 0;
    fread(&iStringSize, 1, 1, pFile);
    static char strTemp[256];
    strTemp[0] = 0;
    fread(strTemp, 1, iStringSize, pFile);
    strTemp[iStringSize] = 0;
#ifdef PROJECT_ANIMATOR
    FILE *pFile = fopen(strTemp, "rb");
    if(pFile != NULL)
    {
      fclose(pFile);
    }
    else
    {
      PickNewSpriteFile(strTemp);
    }
#endif
    m_SpriteFiles[i] = strTemp;
  }

  // read number of animations
  unsigned int iNumAnims = 0;
  fread(&iNumAnims, 1, 4, pFile);

  // read all animations
  m_Animations.resize(iNumAnims);
  for(unsigned int i = 0; i < iNumAnims; i++)
  {
    Animation &anim = m_Animations[i];

    // read name
    unsigned char iStringSize = 0;
    fread(&iStringSize, 1, 1, pFile);
    static char strTemp[256];
    strTemp[0] = 0;
    fread(strTemp, 1, iStringSize, pFile);
    strTemp[iStringSize] = 0;
    anim.strName = strTemp;

    // read duration
    fread(&anim.fDuration, 1, 4, pFile);

    // read number of frames
    unsigned int iFrames = 0;
    fread(&iFrames, 1, 4, pFile);
    anim.Frames.resize(iFrames);

    // read all frames
    for(unsigned int i = 0; i < iFrames; i++)
    {
      // read time
      fread(&anim.Frames[i].fTime, 1, 4, pFile);
      // read poses
      anim.Frames[i].Poses.resize(iNumSprites);
      fread(&anim.Frames[i].Poses[0], 1, sizeof(SpritePose) * iNumSprites, pFile);
    }
  }

  if(iVersion == 2)
  {
    // read number of sprite physics
    unsigned int iNumSpritePhysics = 0;
    fread(&iNumSpritePhysics, 1, 4, pFile);
    m_SpritePhysics.resize(iNumSpritePhysics);
    for(unsigned int i = 0; i < iNumSpritePhysics; i++)
    {
      // read sprite index
      fread(&m_SpritePhysics[i].iSprite, 1, 4, pFile);

      // read number of particles
      unsigned int iNumParticles = 0;
      fread(&iNumParticles, 1, 4, pFile);
      m_SpritePhysics[i].Particles.resize(iNumParticles);

      // read particles
      if(iNumParticles > 0)
        fread(&m_SpritePhysics[i].Particles[0], 1, sizeof(PhysicsParticle) * iNumParticles, pFile);

      // read movepoint particles
      fread(&m_SpritePhysics[i].MovePointParticles, 1, sizeof(ParticleIndex) * 3, pFile);
    }

    // read number of constraints
    unsigned int iNumConstraints = 0;
    fread(&iNumConstraints, 1, 4, pFile);
    m_PhysicsConstraints.resize(iNumConstraints);

    // read constraints
    if(iNumConstraints > 0)
      fread(&m_PhysicsConstraints[0], 1, sizeof(PhysicsConstraint) * iNumConstraints, pFile);
  }

#ifdef _DEBUG
  if(!ValidatePhysics())
    _CrtDbgBreak();
#endif

  fclose(pFile);
  return true;
}

bool AnimationFile::SaveTo(const char *strFile)
{
  FILE *pFile = fopen(strFile, "wb");
  if(pFile == NULL)
  {
    return false;
  }

  // write ID
  char strID[4] = {'A','N','I','M'};
  fwrite(strID, 1, 4, pFile);

  // write version
  unsigned int iVersion = 2;
  fwrite(&iVersion, 1, 4, pFile);

  // write number of sprites
  unsigned int iNumSprites = m_SpriteFiles.size();
  fwrite(&iNumSprites, 1, 4, pFile);

  // write all sprites
  for(unsigned int i = 0; i < iNumSprites; i++)
  {
    unsigned char iStringSize = m_SpriteFiles[i].size();
    fwrite(&iStringSize, 1, 1, pFile);
    fwrite(m_SpriteFiles[i].c_str(), 1, iStringSize, pFile);
  }

  // write number of animations
  unsigned int iNumAnims = m_Animations.size();
  fwrite(&iNumAnims, 1, 4, pFile);

  // write all animations
  for(unsigned int i = 0; i < iNumAnims; i++)
  {
    Animation &anim = m_Animations[i];

    // write name
    unsigned char iStringSize = anim.strName.size();
    fwrite(&iStringSize, 1, 1, pFile);
    fwrite(anim.strName.c_str(), 1, iStringSize, pFile);

    // write duration
    fwrite(&anim.fDuration, 1, 4, pFile);

    // write number of frames
    unsigned int iFrames = anim.Frames.size();
    fwrite(&iFrames, 1, 4, pFile);

    // write all frames
    for(unsigned int i = 0; i < iFrames; i++)
    {
      // write time
      fwrite(&anim.Frames[i].fTime, 1, 4, pFile);
      // write poses
      fwrite(&anim.Frames[i].Poses[0], 1, sizeof(SpritePose) * iNumSprites, pFile);
    }
  }

  // write number of sprite physics
  unsigned int iNumSpritePhysics = m_SpritePhysics.size();
  fwrite(&iNumSpritePhysics, 1, 4, pFile);
  for(unsigned int i = 0; i < iNumSpritePhysics; i++)
  {
    // write sprite index
    fwrite(&m_SpritePhysics[i].iSprite, 1, 4, pFile);

    // write number of particles
    unsigned int iNumParticles = m_SpritePhysics[i].Particles.size();
    fwrite(&iNumParticles, 1, 4, pFile);

    // write particles
    if(iNumParticles > 0)
      fwrite(&m_SpritePhysics[i].Particles[0], 1, sizeof(PhysicsParticle) * iNumParticles, pFile);

    // write movepoint particles
    fwrite(&m_SpritePhysics[i].MovePointParticles, 1, sizeof(ParticleIndex) * 3, pFile);
  }

  // write number of constraints
  unsigned int iNumConstraints = m_PhysicsConstraints.size();
  fwrite(&iNumConstraints, 1, 4, pFile);

  // write constraints
  if(iNumConstraints > 0)
    fwrite(&m_PhysicsConstraints[0], 1, sizeof(PhysicsConstraint) * iNumConstraints, pFile);

  fclose(pFile);
  return true;
}

bool AnimationFile::AddSprite(const char *strFilename, AnimatedSprite *pInstance)
{
  FILE *pFile = fopen(strFilename, "rb");
  if(pFile == NULL)
    return false;
  fclose(pFile);

  m_SpriteFiles.push_back(strFilename);

  SpritePose pose;
  pose.vPosition = Vector3D(0,0,0);
  pose.qRotation = Quaternion::qIdentity;
  pose.vScale = Vector3D(1,1,1);
  for(unsigned int i = 0; i < m_Animations.size(); i++)
  {
    for(unsigned int j = 0; j < m_Animations[i].Frames.size(); j++)
    {
      m_Animations[i].Frames[j].Poses.push_back(pose);
    }
  }

  if(pInstance)
    pInstance->m_Sprites.push_back(new SpritePoser(new Sprite(strFilename)));

  return true;
}

void AnimationFile::DeleteSprite(unsigned int iSprite, AnimatedSprite *pInstance)
{
  if(iSprite >= m_SpriteFiles.size())
    return;

  m_SpriteFiles.erase(m_SpriteFiles.begin() + iSprite);
  for(unsigned int i = 0; i < m_Animations.size(); i++)
  {
    for(unsigned int j = 0; j < m_Animations[i].Frames.size(); j++)
    {
      m_Animations[i].Frames[j].Poses.erase(m_Animations[i].Frames[j].Poses.begin() + iSprite);
    }
  }

  if(pInstance)
  {
    delete pInstance->m_Sprites[iSprite];
    pInstance->m_Sprites.erase(pInstance->m_Sprites.begin() + iSprite);
  }

  // delete physics
  RemoveSpritePhysics(GetSpritePhysicsIndex(iSprite));

  // reset physics indices
  for(unsigned int s = 0; s < m_SpritePhysics.size(); s++)
  {
    SpritePhysics &phys = m_SpritePhysics[s];
    if(phys.iSprite > iSprite)
      phys.iSprite--;
  }
}

unsigned int AnimationFile::GetSprite(const std::string &strFile)
{
  for(unsigned int i = 0; i < m_SpriteFiles.size(); i++)
  {
    if(m_SpriteFiles[i] == strFile)
      return i;
  }
  return -1;
}

unsigned int AnimationFile::AddAnimation(const std::string &strName, float fDuration)
{
  Animation anim;
  anim.fDuration = fDuration;
  anim.strName = strName;
  m_Animations.push_back(anim);
  return m_Animations.size() - 1;
}

unsigned int AnimationFile::GetAnimation(const std::string &strName)
{
  for(unsigned int i = 0; i < m_Animations.size(); i++)
  {
    if(m_Animations[i].strName == strName)
      return i;
  }
  return -1;
}

void AnimationFile::RemoveAnimation(unsigned int iAnim)
{
  if(iAnim < m_Animations.size())
  {
    m_Animations.erase(m_Animations.begin() + iAnim);
  }
}

unsigned int AnimationFile::AddFrame(unsigned int iAnim, float fTime, float fTolerance)
{
  if(iAnim < m_Animations.size())
  {
    Animation &anim = m_Animations[iAnim];

    // find existing
    unsigned int iFrame = GetFrame(iAnim, fTime, fTolerance);
    if(iFrame != -1)
      return iFrame;

    // add new
    Animation::Frame frame;
    frame.fTime = fTime;
    anim.Frames.push_back(frame);

    // default pose
    SpritePose pose;
    pose.vPosition = Vector3D(0,0,0);
    pose.qRotation = Quaternion::qIdentity;
    pose.vScale = Vector3D(1,1,1);
    anim.Frames.back().Poses.resize(m_SpriteFiles.size(), pose);

    SortFrames(iAnim);
    return GetFrame(iAnim, fTime, fTolerance);
  }
  return -1;
}

unsigned int AnimationFile::GetFrame(unsigned int iAnim, float fTime, float fTolerance)
{
  if(iAnim < m_Animations.size())
  {
    Animation &anim = m_Animations[iAnim];

    for(unsigned int iFrame = 0; iFrame < anim.Frames.size(); iFrame++)
    {
      if(fabsf(anim.Frames[iFrame].fTime - fTime) < fTolerance)
      {
        return iFrame;
      }
    }
  }
  return -1;
}

void AnimationFile::RemoveFrame(unsigned int iAnim, unsigned int iFrame)
{
  if(iAnim < m_Animations.size())
  {
    if(iFrame < m_Animations[iAnim].Frames.size())
    {
      m_Animations[iAnim].Frames.erase(m_Animations[iAnim].Frames.begin() + iFrame);
    }
  }
}

void AnimationFile::SortFrames(unsigned int iAnim)
{
  struct FrameSort
  {
    bool operator()(Animation::Frame &a, Animation::Frame &b) { return a.fTime < b.fTime; }
  };

  if(iAnim < m_Animations.size())
  {
    std::sort(m_Animations[iAnim].Frames.begin(), m_Animations[iAnim].Frames.end(), FrameSort());
  }
}

void AnimationFile::SetPoses(unsigned int iAnim, unsigned int iFrame, const std::vector<unsigned int> &SpriteIndices, const std::vector<SpritePoser *> &Sprites)
{
 if(iAnim >= m_Animations.size())
    return;
  Animation &anim = m_Animations[iAnim];
  if(iFrame >= anim.Frames.size())
    return;
  Animation::Frame &frame = anim.Frames[iFrame];
  std::vector<SpritePose> &poses = frame.Poses;

  if(Sprites.size() != SpriteIndices.size())
  {
    printf("AnimationFile::SetPoses needs index for each sprite\n");
    return;
  }

  // copy to indexed pose
  for(unsigned int i = 0; i < SpriteIndices.size(); i++)
  {
    unsigned int iIndex = SpriteIndices[i];
    if(iIndex < poses.size())
    {
      poses[iIndex].vPosition = Sprites[i]->m_vPosition;
      poses[iIndex].qRotation = Sprites[i]->m_qRotation;
      poses[iIndex].vScale = Sprites[i]->m_vScale;
    }
  }
}

void AnimationFile::SetPoses(unsigned int iAnim, unsigned int iFrame, const std::vector<SpritePoser *> &Sprites)
{
  if(iAnim >= m_Animations.size())
    return;
  Animation &anim = m_Animations[iAnim];
  if(iFrame >= anim.Frames.size())
    return;
  Animation::Frame &frame = anim.Frames[iFrame];
  std::vector<SpritePose> &poses = frame.Poses;

  if(Sprites.size() != m_SpriteFiles.size())
  {
    printf("AnimationFile::SetPoses needs all sprites (%i instead of %i) as input!\n", m_SpriteFiles.size(), Sprites.size());
    return;
  }

  // direct copy
  poses.resize(Sprites.size());
  for(unsigned int i = 0; i < Sprites.size(); i++)
  {
    poses[i].vPosition = Sprites[i]->m_vPosition;
    poses[i].qRotation = Sprites[i]->m_qRotation;
    poses[i].vScale = Sprites[i]->m_vScale;
  }
}

AnimatedSprite *AnimationFile::CreateNewInstance(void)
{
  AnimatedSprite *pInstance = new AnimatedSprite(this);
  return pInstance;
}

unsigned int AnimationFile::AddSpritePhysics(unsigned int iSprite)
{
  // already exists
  unsigned int i = GetSpritePhysicsIndex(iSprite);
  if(i != -1)
    return i;

  SpritePhysics dummy;
  dummy.iSprite = iSprite;
  dummy.MovePointParticles[0] = ParticleIndex(-1,-1);
  dummy.MovePointParticles[1] = ParticleIndex(-1,-1);
  dummy.MovePointParticles[2] = ParticleIndex(-1,-1);
  m_SpritePhysics.push_back(dummy);
  return m_SpritePhysics.size() - 1;
}

unsigned int AnimationFile::GetSpritePhysicsIndex(unsigned int iSprite)
{
  for(unsigned int i = 0; i < m_SpritePhysics.size(); i++)
    if(m_SpritePhysics[i].iSprite == iSprite)
      return i;
  return -1;
}

void AnimationFile::RemoveParticle(unsigned int iSpritePhysics, unsigned int iParticle)
{
  ParticleIndex Remove(iSpritePhysics, iParticle);

  // remove the particle
  SpritePhysics &phys_r = m_SpritePhysics[Remove.iSpritePhysics];
  phys_r.Particles.erase(phys_r.Particles.begin() + Remove.iParticle);

  // reset all movepoints
  for(unsigned int s = 0; s < m_SpritePhysics.size(); s++)
  {
    SpritePhysics &phys = m_SpritePhysics[s];

    bool bCollapseOthers = false;
    for(int j = 0; j < 3; j++)
    {
      // pointing to this particle
      if(phys.MovePointParticles[j] == Remove)
      {
        phys.MovePointParticles[j].iParticle = -1;
        phys.MovePointParticles[j].iSpritePhysics = -1;
        bCollapseOthers = true;
      }
      else if(bCollapseOthers)
      {
        phys.MovePointParticles[j-1] = phys.MovePointParticles[j];
        phys.MovePointParticles[j].iParticle = -1;
        phys.MovePointParticles[j].iSpritePhysics = -1;
      }
    }
    for(int j = 0; j < 3; j++)
    {
      // pointing to particle above this
      if(phys.MovePointParticles[j].iSpritePhysics == Remove.iSpritePhysics
        && phys.MovePointParticles[j].iParticle > Remove.iParticle)
      {
        // reduce index
        phys.MovePointParticles[j].iParticle--;
      }
    }
  }

  // remove constraints
  for(unsigned int c = 0; c < m_PhysicsConstraints.size();)
  {
    PhysicsConstraint &constraint = m_PhysicsConstraints[c];

    // pointing to this particle 
    if((constraint.iSpritePhysics1 == Remove.iSpritePhysics && constraint.iParticle1 == Remove.iParticle) ||
       (constraint.iSpritePhysics2 == Remove.iSpritePhysics && constraint.iParticle2 == Remove.iParticle))
    {
      // remove
      m_PhysicsConstraints.erase(m_PhysicsConstraints.begin() + c);
    }
    else
    {
      // pointing to particle above this - reduce index
      if(constraint.iSpritePhysics1 == Remove.iSpritePhysics && constraint.iParticle1 > Remove.iParticle)
        constraint.iParticle1--;
      // pointing to particle above this - reduce index
      if(constraint.iSpritePhysics2 == Remove.iSpritePhysics && constraint.iParticle2 > Remove.iParticle)
        constraint.iParticle2--;

      c++;
    }
  }

#ifdef _DEBUG
  if(!ValidatePhysics())
    _CrtDbgBreak();
#endif
}

void AnimationFile::RemoveSpritePhysics(unsigned int iSpritePhysics)
{
  if(iSpritePhysics >= m_SpritePhysics.size())
    return;

  // reset all movepoints
  for(unsigned int s = 0; s < m_SpritePhysics.size(); s++)
  {
    SpritePhysics &phys = m_SpritePhysics[s];

    bool bCollapseOthers = false;
    for(int j = 0; j < 3; j++)
    {
      // pointing to this sprite
      if(phys.MovePointParticles[j].iSpritePhysics == iSpritePhysics)
      {
        phys.MovePointParticles[j].iParticle = -1;
        phys.MovePointParticles[j].iSpritePhysics = -1;
        bCollapseOthers = true;
      }
      else if(bCollapseOthers)
      {
        phys.MovePointParticles[j-1] = phys.MovePointParticles[j];
        phys.MovePointParticles[j].iParticle = -1;
        phys.MovePointParticles[j].iSpritePhysics = -1;
      }
    }
    for(int j = 0; j < 3; j++)
    {
      // pointing to sprite above this - reduce index
      if(phys.MovePointParticles[j].IsSet() && phys.MovePointParticles[j].iSpritePhysics > iSpritePhysics)
        phys.MovePointParticles[j].iSpritePhysics--;
    }
  }

  // remove constraints
  for(unsigned int c = 0; c < m_PhysicsConstraints.size();)
  {
    PhysicsConstraint &constraint = m_PhysicsConstraints[c];
    // pointing to this sprite
    if(constraint.iSpritePhysics1 == iSpritePhysics || constraint.iSpritePhysics2 == iSpritePhysics)
    {
      // remove
      m_PhysicsConstraints.erase(m_PhysicsConstraints.begin() + c);
    }
    else
    {
      // pointing to sprite above this - reduce index
      if(constraint.iSpritePhysics1 != -1 && constraint.iSpritePhysics1 > iSpritePhysics)
        constraint.iSpritePhysics1--;
      // pointing to sprite above this - reduce index
      if(constraint.iSpritePhysics2 != -1 && constraint.iSpritePhysics2 > iSpritePhysics)
        constraint.iSpritePhysics2--;

      c++;
    }
  }

  m_SpritePhysics.erase(m_SpritePhysics.begin() + iSpritePhysics);

#ifdef _DEBUG
  if(!ValidatePhysics())
    _CrtDbgBreak();
#endif
}

unsigned int AnimationFile::ChangeParticleSpritePhysics(unsigned int iSpritePhysics, unsigned int iParticle, unsigned int iNewSpritePhysics)
{
  // no change!
  if(iSpritePhysics == iNewSpritePhysics)
    return iSpritePhysics;

  // add to new
  unsigned int iNewParticle = m_SpritePhysics[iNewSpritePhysics].Particles.size();
  m_SpritePhysics[iNewSpritePhysics].Particles.push_back( m_SpritePhysics[iSpritePhysics].Particles[iParticle] );

  // remove the old
  m_SpritePhysics[iSpritePhysics].Particles.erase(m_SpritePhysics[iSpritePhysics].Particles.begin() + iParticle);


  ParticleIndex OldParticle(iSpritePhysics, iParticle);
  ParticleIndex NewParticle(iNewSpritePhysics, iNewParticle);

  // reset all movepoints
  for(unsigned int s = 0; s < m_SpritePhysics.size(); s++)
  {
    SpritePhysics &phys = m_SpritePhysics[s];
    for(int j = 0; j < 3; j++)
    {
      // pointing to old particle
      if(phys.MovePointParticles[j] == OldParticle)
      {
        // change to new
        phys.MovePointParticles[j] = NewParticle;
      }
      // pointing to particle above old
      else if(phys.MovePointParticles[j].iSpritePhysics == iSpritePhysics && phys.MovePointParticles[j].iParticle > iParticle)
      {
        // reduce index
        phys.MovePointParticles[j].iParticle--;
      }
    }
  }

  // fix constraints
  for(unsigned int c = 0; c < m_PhysicsConstraints.size();)
  {
    PhysicsConstraint &constraint = m_PhysicsConstraints[c];

    // pointing to old particle 
    if(constraint.iSpritePhysics1 == iSpritePhysics && constraint.iParticle1 == iParticle)
    {
      // change to new
      constraint.iSpritePhysics1 = iNewSpritePhysics;
      constraint.iParticle1 = iNewParticle;
    }
    // pointing to old particle 
    else if(constraint.iSpritePhysics2 == iSpritePhysics && constraint.iParticle2 == iParticle)
    {
      // change to new
      constraint.iSpritePhysics2 = iNewSpritePhysics;
      constraint.iParticle2 = iNewParticle;
    }
    else
    {
      // pointing to particle above old - reduce index
      if(constraint.iSpritePhysics1 == iSpritePhysics && constraint.iParticle1 > iParticle)
        constraint.iParticle1--;
      // pointing to particle above this - reduce index
      if(constraint.iSpritePhysics2 == iSpritePhysics && constraint.iParticle2 > iParticle)
        constraint.iParticle2--;

      c++;
    }
  }

#ifdef _DEBUG
  if(!ValidatePhysics())
    _CrtDbgBreak();
#endif

  return iNewParticle;
}

void AnimationFile::RemoveEmptySpritePhysics(void)
{
  for(unsigned int r = 0; r < m_SpritePhysics.size();)
  {
    SpritePhysics &phys_r = m_SpritePhysics[r];
    if(phys_r.Particles.empty()
      && !phys_r.MovePointParticles[0].IsSet()
      && !phys_r.MovePointParticles[1].IsSet()
      && !phys_r.MovePointParticles[2].IsSet())
    {
      RemoveSpritePhysics(r);
    }
    else
    {    
      r++;
    }
  }

#ifdef _DEBUG
  if(!ValidatePhysics())
    _CrtDbgBreak();
#endif
}

bool AnimationFile::ValidatePhysics(void)
{
  // validate constraints
  for(unsigned int i = 0; i < m_PhysicsConstraints.size(); i++)
  {
    PhysicsConstraint &constraint = m_PhysicsConstraints[i];
    if(constraint.iSpritePhysics1 >= m_SpritePhysics.size())
      return false;
    if(constraint.iSpritePhysics2 >= m_SpritePhysics.size())
      return false;

    if(constraint.iParticle1 >= m_SpritePhysics[constraint.iSpritePhysics1].Particles.size())
      return false;
    if(constraint.iParticle2 >= m_SpritePhysics[constraint.iSpritePhysics2].Particles.size())
      return false;
  }

  for(unsigned int i = 0; i < m_SpritePhysics.size(); i++)
  {
    SpritePhysics &phys = m_SpritePhysics[i];
    if(phys.iSprite >= m_SpriteFiles.size())
      return false;

    for(int j = 0; j < 3; j++)
    {
      if(phys.MovePointParticles[j].IsSet())
      {
        if(phys.MovePointParticles[j].iSpritePhysics >= m_SpritePhysics.size())
          return false;
        if(phys.MovePointParticles[j].iParticle >= m_SpritePhysics[ phys.MovePointParticles[j].iSpritePhysics ].Particles.size())
          return false;
      }
    }
  }

  return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////

AnimationFileManager::AnimationFileManager()
{
}

AnimationFileManager::~AnimationFileManager()
{
  std::map<std::string, AnimationFile *>::iterator it;
  for(it = m_Files.begin(); it != m_Files.end(); it++)
  {
    delete it->second;
  }
}

AnimationFileManager *AnimationFileManager::GetInstance(void)
{
  static AnimationFileManager _amf;
  return &_amf;
}

AnimationFile *AnimationFileManager::LoadFile(const char *strFile)
{
  std::map<std::string, AnimationFile *>::iterator it = m_Files.find(strFile);
  if(it != m_Files.end())
  {
    return it->second;
  }
  AnimationFile *pFile = new AnimationFile();
  if(!pFile->LoadFrom(strFile))
  {
    delete pFile;
    return NULL;
  }
  m_Files[strFile] = pFile;
  return pFile;
}

AnimatedSprite *AnimationFileManager::CreateFromFile(const char *strFile)
{
  AnimationFile *pFile = LoadFile(strFile);
  if(pFile == NULL)
    return NULL;

  return pFile->CreateNewInstance();
}
