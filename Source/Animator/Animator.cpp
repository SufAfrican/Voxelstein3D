#include "StdAfx.h"
#include "Engine/Core/Collision.h"
#include "Animator/AnimatorTool.h"
#include "Animator/Resources.h"
#include "Animator/CameraTool.h"
#include "Animator/SelectionTool.h"
#include "Animator/RotateTool.h"
#include "Animator/MoveTool.h"
#include "Animator/ScaleTool.h"
#include "Animator/StringInputDialog.h"

#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"

#include <commdlg.h>
#include <direct.h>

static const float fParticlePoserSpriteRadius = 8.0f;

extern HWND ghwnd;

Animator::Animator()
{
  m_mCamera = Matrix44::mIdentity;
  m_vMouse = Vector2D(0,0);
  m_bMouseLeft = false;
  m_bMouseRight = false;
  m_pCameraTool = new CameraTool();
  m_pAnimWindow = NULL;

  m_pFile = NULL;
  m_pInstance = NULL;
  m_bFileChanged = false;

  m_Tools.resize(4);
  m_Tools[0] = new SelectionTool();
  m_Tools[1] = new MoveTool();
  m_Tools[2] = new RotateTool();
  m_Tools[3] = new ScaleTool();

  const float fTicksPerSecond = 50.0f;
  m_fTickDuration = 1.0f / fTicksPerSecond;
  m_fTime = Time::GetTime();
  m_fLastTick = m_fTime;

  m_bAutoSetFrame = true;

  g_pPhysicsSolver = new PhysicsSolver();
  m_bPhysicsMode = false;

  m_iSpritePicking = 0;

  m_bRenderGrid = true;

  NewFile();
}

Animator::~Animator()
{
  for(unsigned int i = 0; i < m_ParticlePosers.size(); i++)
    delete m_ParticlePosers[i];
  delete m_pAnimWindow;
  for(unsigned int i = 0; i < m_Tools.size(); i++)
    delete m_Tools[i];
  delete m_pCameraTool;
  delete m_pInstance;
  delete m_pFile;
  delete g_pPhysicsSolver;
}

void Animator::CreateWindows(void)
{
  m_pAnimWindow = new AnimWindow();
}

bool Animator::IsMouseLeftDown(void)
{
  if(GetFocus() != ghwnd)
    return false;
  return m_bMouseLeft;
}

bool Animator::IsMouseRightDown(void)
{
  if(GetFocus() != ghwnd)
    return false;
  return m_bMouseRight;
}

bool Animator::IsKeyClicked(int iVirtualKey)
{
  if(GetFocus() != ghwnd)
    return false;

  static std::map<int, float> KeyAlreadyDown;
  std::map<int, float>::iterator it = KeyAlreadyDown.find(iVirtualKey);

  if( ((GetKeyState(iVirtualKey) & 0xfe) != 0) )
  {
    if(it == KeyAlreadyDown.end()
      || (m_fTime - it->second) > 0.5f) // 0.5 second repeat delay
    {
      KeyAlreadyDown[iVirtualKey] = m_fTime;
      return true;
    }
  }
  else
  {
    if(it != KeyAlreadyDown.end())
      KeyAlreadyDown.erase(it);
  }
  return false;
}

bool Animator::IsKeyDown(int iVirtualKey, bool bInMainWindow, bool bInAnimWindow)
{
  bool bIsInMainWindow = GetFocus() == ghwnd;
  bool bIsInAnimWindow = GetFocus() == m_pAnimWindow->m_hWindow;

  if(!bIsInMainWindow && !bIsInAnimWindow)
    return false;

  if(bIsInAnimWindow && !bInAnimWindow)
    return false;
  
  if(bIsInMainWindow && !bIsInMainWindow)
    return false;

  return ((GetKeyState(iVirtualKey) & 0xfe) != 0);
}

void Animator::DrawGrid(void)
{
  if(!m_bRenderGrid)
    return;

  // draw grid
  Vector3D vCamera = m_mCamera.GetTranslation();
  Vector3D vTarget = Vector3D(0,0,0);

  float fDistance = (vCamera - vTarget).Length();
  if(fDistance < 0.1f)
    fDistance = 0.1f;
  float fGridSize = GetNearestPowerOfTwo(fDistance / 10.0f);

  Vector3D vMin = vTarget - Vector3D(fDistance, 0, fDistance);
  Vector3D vMax = vTarget + Vector3D(fDistance, 0, fDistance);

  vMin.x = floorf(vMin.x / fGridSize) * fGridSize;
  vMin.z = floorf(vMin.z / fGridSize) * fGridSize;

  vMax.x = ceilf(vMax.x / fGridSize) * fGridSize;
  vMax.z = ceilf(vMax.z / fGridSize) * fGridSize;

  unsigned int iGridColor = 0x60;
  unsigned int iMainLineColor = 0x80;
  for(float fX = vMin.x; fX < vMax.x; fX += fGridSize)
  {
    for(float fY = vMin.z; fY < vMax.z; fY += fGridSize)
    {
      unsigned int iColor;

      iColor = fY == 0 ? iMainLineColor : iGridColor;
      DrawLine(Vector3D(fX, 0, fY), Vector3D(fX + fGridSize, 0, fY), iColor,iColor,iColor);

      iColor = fX == 0 ? iMainLineColor : iGridColor;
      DrawLine(Vector3D(fX, 0, fY), Vector3D(fX, 0, fY + fGridSize), iColor,iColor,iColor);
    }
  }

  DrawLine(Vector3D(vMax.x, 0, vMax.z), Vector3D(vMax.x, 0, vMin.z), iGridColor,iGridColor,iGridColor);
  DrawLine(Vector3D(vMax.x, 0, vMax.z), Vector3D(vMin.x, 0, vMax.z), iGridColor,iGridColor,iGridColor);


  // draw axis
  DrawArrow(Vector3D(1,0,1), Vector3D(1,0,50), 2.0f, Vector3D(0,1,0), 0,0,128);
  DrawArrow(Vector3D(1,0,1), Vector3D(1,50,1), 2.0f, Vector3D(0,0,1), 0,128,0);
  DrawArrow(Vector3D(1,0,1), Vector3D(50,0,1), 2.0f, Vector3D(0,1,0), 128,0,0);
}

void Animator::DrawSpritePickingVisualization(SpritePoser *pSprite)
{
  Vector3D vMin,vMax;
  pSprite->GetSprite()->CalculateBoundingBox(vMin,vMax, true, true);
  DrawBoxLines(vMin, vMax, 255,255,0, false);

  // find sprite index
  unsigned int iIndex;
  if(!GetSpriteIndex(pSprite, iIndex))
    return;
  
  // draw visualization
  for(unsigned int i = 0; i < m_pFile->m_SpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[i];
    if(phys.iSprite != iIndex)
      continue;

    // visualize parented particles
    for(unsigned int p = 0; p < phys.Particles.size(); p++)
    {
      AnimationFile::PhysicsParticle &particle = phys.Particles[p];
      Vector3D vPos = Transform(m_pInstance->m_Sprites[phys.iSprite]->GetSprite()->GetTransform(), particle.vPosition);
      DrawCircle(vPos, Vector3D(1,0,0), Vector3D(0,1,0), 1.0f, 10, 255,0,0, false);
      DrawCircle(vPos, Vector3D(0,0,1), Vector3D(0,1,0), 1.0f, 10, 255,0,0, false);
    }

    // visualize movepoints
    for(int j = 0; j < 3; j++)
    {
      AnimationFile::ParticleIndex &index = phys.MovePointParticles[j];
      if(!index.IsSet())
        continue;

      AnimationFile::SpritePhysics &phys_mp = m_pFile->m_SpritePhysics[ index.iSpritePhysics ];
      AnimationFile::PhysicsParticle &particle_mp = phys_mp.Particles[ index.iParticle ];

      int iR = index.iSpritePhysics == i ? 255 : 0;

      Vector3D vPos = Transform(m_pInstance->m_Sprites[phys_mp.iSprite]->GetSprite()->GetTransform(), particle_mp.vPosition);
      DrawCircle(vPos, Vector3D(1,0,0), Vector3D(0,1,0), 1.0f, 10, iR,255,255-iR, false);
      DrawCircle(vPos, Vector3D(0,0,1), Vector3D(0,1,0), 1.0f, 10, iR,255,255-iR, false);
    }
  }
}

void Animator::Run(float fDeltaTime)
{
  m_fTime = Time::GetTime();

  m_pCameraTool->Run(fDeltaTime);

  extern void drawsprites();
  drawsprites();

  DrawGrid();

  if(!m_pInstance->IsRagdoll())
  {
    // picking sprites in physics mode
    if(m_bPhysicsMode && m_iSpritePicking > 0)
    {
      // run only select tool
      m_Tools[0]->Run(fDeltaTime);
    }
    else
    {
      for(unsigned int i = 0; i < m_Tools.size(); i++)
      {
        m_Tools[i]->Run(fDeltaTime);
      }
    }
  }

  bool bPlayAnim = m_pInstance->m_bPlaying;
  m_pInstance->Run(fDeltaTime, bPlayAnim);
  
  if(bPlayAnim)
  {
    // update timeline
    m_pAnimWindow->UpdateTimeLine();
    // autostop
    if(bPlayAnim && !m_pInstance->m_bLooping && m_pInstance->m_fCurrentTime >= 1.0f)
      m_pInstance->m_bPlaying = false;
  }

  // draw selection
  std::vector<SpritePoser *> &Selected = m_bPhysicsMode ? m_SelectedParticles : m_Selected;
  for(unsigned int i = 0; i < Selected.size(); i++)
  {
    Vector3D vMin,vMax;
    Selected[i]->GetSprite()->CalculateBoundingBox(vMin,vMax, true, true);
    int iR = 255;
    int iG = 255;
    int iB = 0;
    if(m_iSpritePicking > 0)
    {
      iG = 0;
    }
    DrawBoxLines(vMin, vMax, iR,iG,iB);
  }

  // physics mode
  if(m_bPhysicsMode && m_iSpritePicking == 0)
  {
    std::vector<AnimationFile::SpritePhysics> &SpritePhysics = m_pFile->m_SpritePhysics;

    // update particle settings
    for(unsigned int i = 0; i < m_ParticlePosers.size(); i++)
    {
      ParticlePoserLink &link = m_ParticlePoserLinks[GetParticleLinkIndex(m_ParticlePosers[i])];
      AnimationFile::SpritePhysics &phys = SpritePhysics[link.iSpritePhysics];
      AnimationFile::PhysicsParticle &particle = phys.Particles[link.iParticle];
      Matrix44 mInvSprite = Inverse(m_pInstance->m_Sprites[phys.iSprite]->GetSprite()->GetTransform());

      particle.vPosition = Transform(mInvSprite, m_ParticlePosers[i]->m_vPosition);
      particle.fRadius = Max(fabsf(m_ParticlePosers[i]->m_vScale.x), 1.0f/fParticlePoserSpriteRadius) * Sign(particle.fRadius) * fParticlePoserSpriteRadius;

      m_ParticlePosers[i]->m_vScale = Vector3D(fabsf(particle.fRadius) / fParticlePoserSpriteRadius, fabsf(particle.fRadius) / fParticlePoserSpriteRadius, fabsf(particle.fRadius) / fParticlePoserSpriteRadius);
      m_ParticlePosers[i]->m_qRotation = Quaternion::qIdentity;
      m_ParticlePosers[i]->UpdateSprite(Matrix44::mIdentity);
    }
    
    // visualize
    for(unsigned int i = 0; i < SpritePhysics.size(); i++)
    {
      AnimationFile::SpritePhysics &phys = SpritePhysics[i];     
      for(unsigned int p = 0; p < phys.Particles.size(); p++)
      {
        AnimationFile::PhysicsParticle &particle = phys.Particles[p];
        Vector3D vPos = Transform(m_pInstance->m_Sprites[phys.iSprite]->GetSprite()->GetTransform(), particle.vPosition);

        int iR = 255;
        int iG = 0;
        int iB = 0;

        // colorize selected
        for(unsigned int j = 0; j < m_SelectedParticles.size(); j++)
        {
          ParticlePoserLink &link = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[j])];
          if(link.iSpritePhysics == i && link.iParticle == p)
          {
            iG = 255;
            break;
          }
        }

        // colorize movepoint particles
        if(iG == 0) // except if already selected
        {
          for(unsigned int i2 = 0; i2 < SpritePhysics.size(); i2++)
          {
            int k;
            for(k = 0; k < 3; k++)
            {
              if(SpritePhysics[i2].MovePointParticles[k].iSpritePhysics != i)
                continue;
              if(SpritePhysics[i2].MovePointParticles[k].iParticle == p)
              {
                iR = 0;
                iB = 255;
                iG = 255;
                break;
              }
            }
            if(k < 3)
              break;
          }
        }

        // draw particles
        DrawCircle(vPos, Vector3D(1,0,0), Vector3D(0,1,0), 1.0f, 10, iR,iG,iB, false);
        DrawCircle(vPos, Vector3D(0,0,1), Vector3D(0,1,0), 1.0f, 10, iR,iG,iB, false);
      }
    }
    // draw constraints
    for(unsigned int i = 0; i < m_pFile->m_PhysicsConstraints.size(); i++)
    {
      AnimationFile::PhysicsConstraint &constraint = m_pFile->m_PhysicsConstraints[i];
      AnimationFile::SpritePhysics &phys1 = SpritePhysics[constraint.iSpritePhysics1];
      AnimationFile::SpritePhysics &phys2 = SpritePhysics[constraint.iSpritePhysics2];
      AnimationFile::PhysicsParticle &particle1 = phys1.Particles[constraint.iParticle1];
      AnimationFile::PhysicsParticle &particle2 = phys2.Particles[constraint.iParticle2];

      Vector3D vPos1 = Transform(m_pInstance->m_Sprites[phys1.iSprite]->GetSprite()->GetTransform(), particle1.vPosition);
      Vector3D vPos2 = Transform(m_pInstance->m_Sprites[phys2.iSprite]->GetSprite()->GetTransform(), particle2.vPosition);

      if(constraint.fDistance == 0.0f)
      {
        DrawLine(vPos1, vPos2, 255,0,255, false);
      }
      else
      {
        DrawLine(vPos1, vPos2, 0,255,0, false);
        Vector3D vDir = Normalize(vPos2 - vPos1);
        Vector3D vCenter = (vPos1+vPos2)*0.5f;
        DrawLine(vCenter - constraint.fDistance * vDir * 0.5f, vCenter + constraint.fDistance * vDir * 0.5f, 255,0,255, false);
      }
    }
  }


  // shortcut keys
  if(IsKeyDown(VK_CONTROL, true, true))
  {
    if(IsKeyDown('C', true, true))
      CopyPoses();
    else if(IsKeyDown('V', true, true))
      PastePoses();
  }

  // is in physics preview mode
  if(m_pInstance->IsRagdoll())
  {
    // draw particles
    for(unsigned int i = 0; i < GetPhysics()->m_ActiveParticles.size(); i++)
    {
      PhysicsSolver::Particle &particle = GetPhysics()->m_Particles[ GetPhysics()->m_ActiveParticles[i] ];
      int iG = particle.IsSleeping() ? 255 : 0;
      DrawCircle(particle.vPos, Vector3D(1,0,0), Vector3D(0,1,0), 1.0f, 10, 255,iG,0, false);
      DrawCircle(particle.vPos, Vector3D(0,0,1), Vector3D(0,1,0), 1.0f, 10, 255,iG,0, false);
    }

    // draw constraints
    for(unsigned int i = 0; i < GetPhysics()->m_ActiveConstraints.size(); i++)
    {
      PhysicsSolver::Constraint &constraint = GetPhysics()->m_Constraints[ GetPhysics()->m_ActiveConstraints[i] ];
      PhysicsSolver::Particle &particle1 = GetPhysics()->m_Particles[constraint.iP1];
      PhysicsSolver::Particle &particle2 = GetPhysics()->m_Particles[constraint.iP2];

      if(constraint.fLength > 0.0f)
      {
        DrawLine(particle1.vPos, particle2.vPos, 255,0,255, false);
      }
      else
      {
        DrawLine(particle1.vPos, particle2.vPos, 0,255,0, false);
        Vector3D vDir = Normalize(particle2.vPos - particle1.vPos);
        Vector3D vCenter = (particle1.vPos+particle2.vPos)*0.5f;
        DrawLine(vCenter - constraint.fLength * vDir * 0.5f, vCenter + constraint.fLength * vDir * 0.5f, 255,0,255, false);
      }
    }

    if(IsKeyDown(VK_ESCAPE, true, true))
      TogglePhysicsPreviewMode();
  }

  if(IsKeyDown(VK_SHIFT))
    m_fTickDuration = 1.0f / 5.0f;
  else
    m_fTickDuration = 1.0f / 50.0f;

  // update physics with constant tick rate
  int iSafety = 0;
  while(m_fTime - m_fLastTick >= m_fTickDuration)
  {
    g_pPhysicsSolver->Tick();

    m_fLastTick += m_fTickDuration;
    iSafety++;

    // game was hanging, so let's just skip whatever time has passed...
    if(iSafety > 10)
    {
      m_fLastTick = m_fTime;
      break;
    }
  }
}

Vector3D Animator::GetCursorPosition(void)
{
  POINT point;
  GetCursorPos(&point);
  ScreenToClient(ghwnd, &point);
  return GetWorldFromScreen(point.x, point.y, m_mCamera);
}

void Animator::Select(SpritePoser *pSprite)
{
  std::vector<SpritePoser *> &Selected = m_bPhysicsMode && m_iSpritePicking == 0 ? m_SelectedParticles : m_Selected;

  if(pSprite == NULL)
    Selected.clear();
  else
  {
    // remove existing
    for(unsigned int i = 0; i < Selected.size(); i++)
    {
      if(Selected[i] == pSprite)
      {
        Selected.erase(Selected.begin() + i);
        return;
      }
    }

    // add new
    Selected.push_back(pSprite);
  }

  // handle sprite picking
  switch(m_iSpritePicking)
  {
  case SpritePicking_SetParticleAsMovePoint: SetParticleAsMovePoint_internal(); break;
  case SpritePicking_AddParticle: AddParticle_internal(); break;
  case SpritePicking_SetParticleParent: SetParticleParent_internal(); break;
  case SpritePicking_RemoveMovePointsFromSprite: RemoveMovePointsFromSprite_internal(); break;
  };
  m_iSpritePicking = 0;
}

void Animator::SelectAll(void)
{
  if(m_bPhysicsMode)
    m_SelectedParticles = m_ParticlePosers;
  else
    m_Selected = m_pInstance->m_Sprites;
}

void Animator::UnloadFile(void)
{
  if(m_bPhysicsMode)
    TogglePhysicsMode();
  delete m_pInstance;
  delete m_pFile;
  m_pFile = NULL;
  m_pInstance = NULL;
  m_Selected.clear();
  m_SaveSprites.clear();
}

void Animator::NewFile(void)
{
  if(m_bFileChanged)
  {
    int iRes = MessageBox(NULL, "The file has been changed. Do you want to save changes?", "Animator", MB_ICONWARNING|MB_YESNOCANCEL);
    if(iRes == IDYES)
    {
      if(!SaveFile()) return;
    }
    else if(iRes == IDCANCEL)
    {
      return;
    }
  }

  UnloadFile();
  m_pFile = new AnimationFile();
  m_pFile->AddAnimation("Untitled", 1.0f);
  m_pInstance = m_pFile->CreateNewInstance();
  m_pInstance->m_iCurrentAnim = 0;
  m_bFileChanged = false;
  if(m_pAnimWindow)
    m_pAnimWindow->Update();
}

static char _strDialogFile[MAX_PATH] = {0};

void Animator::OpenFile(void)
{
  if(m_bFileChanged)
  {
    int iRes = MessageBox(NULL, "The file has been changed. Do you want to save changes?", "Animator", MB_ICONWARNING|MB_YESNOCANCEL);
    if(iRes == IDYES)
    {
      if(!SaveFile()) return;
    }
    else if(iRes == IDCANCEL)
    {
      return;
    }
  }

  if(_strDialogFile[0] == 0)
  {
    GetModuleFileName(GetModuleHandle(NULL),_strDialogFile,MAX_PATH);
    std::string strPath = GetPath(_strDialogFile) + "\\*.anim";
    strcpy(_strDialogFile, strPath.c_str());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = _strDialogFile;
  ofn.nMaxFile = 260;
  ofn.lpstrFilter = TEXT("Animation (.anim)\0*.anim\0");
  ofn.lpstrDefExt = TEXT("anim");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
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
    UnloadFile();
    m_pFile = new AnimationFile();
    if(!m_pFile->LoadFrom(_strDialogFile))
    {
      MessageBox(NULL, "Opening file failed!", "Error!", MB_OK);
      UnloadFile();
      NewFile();
    }
    else
    {
      unsigned int iParticles = 0;
      for(unsigned int i = 0; i < m_pFile->m_SpritePhysics.size(); i++)
        iParticles += m_pFile->m_SpritePhysics[i].Particles.size();

      unsigned int iFrames = 0;
      for(unsigned int i = 0; i < m_pFile->m_Animations.size(); i++)
        iFrames += m_pFile->m_Animations[i].Frames.size();

      printf("Loaded %s\n  %i sprites\n  %i animations, total %i frames\n  %i ragdoll sprites, total %i particles and %i constraints\n",
        _strDialogFile,
        m_pFile->m_SpriteFiles.size(),
        m_pFile->m_Animations.size(),
        iFrames,
        m_pFile->m_SpritePhysics.size(),
        iParticles,
        m_pFile->m_PhysicsConstraints.size());
      m_pInstance = m_pFile->CreateNewInstance();
      m_pAnimWindow->Update();
    }
    m_bFileChanged = false;
    m_strFilename = _strDialogFile;
  }
}

bool Animator::SaveFile(void)
{
  if(m_strFilename.empty())
  {
    return SaveFileAs();
  }

  if(m_pFile->SaveTo(m_strFilename.c_str()))
  {
    m_bFileChanged = false;

    for(unsigned int i = 0; i < m_SaveSprites.size(); i++)
    {
      const char *strName = m_SaveSprites[i].c_str();
      savekv6(strName, getkv6(strName));
    }
    m_SaveSprites.clear();
    return true;
  }
  else
  {
    MessageBox(NULL, "Saving file failed!", "Error!", MB_OK);
  }
  
  return false;
}

bool Animator::SaveFileAs(void)
{
  if(_strDialogFile[0] == 0)
  {
    GetModuleFileName(GetModuleHandle(NULL),_strDialogFile,MAX_PATH);
    std::string strPath = GetPath(_strDialogFile) + "\\*.anim";
    strcpy(_strDialogFile, strPath.c_str());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = _strDialogFile;
  ofn.nMaxFile = 260;
  ofn.lpstrFilter = TEXT("Animation (.anim)\0*.anim\0");
  ofn.lpstrDefExt = TEXT("anim");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
  ofn.hwndOwner = ghwnd;
  char strDir[MAX_PATH];
  GetModuleFileName(GetModuleHandle(NULL),strDir,MAX_PATH);
  ofn.lpstrInitialDir = strDir;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

  if(GetSaveFileName(&ofn))
  {
    std::string strOldFilename = m_strFilename;
    m_strFilename = _strDialogFile;
    if(SaveFile())
    {
      return true;
    }
    else
    {
      // restore old filename
      m_strFilename = strOldFilename;
      return false;
    }
  }
  return false;
}

bool Animator::CanExit(void)
{
  if(m_bFileChanged)
  {
    int iRes = MessageBox(NULL,"The file has been changed. Do you want to save changes?", "Animator", MB_ICONWARNING|MB_YESNOCANCEL);
    if(iRes == IDYES)
    {
      if(!SaveFile()) return false;
    }
    else if(iRes == IDCANCEL)
    {
      return false;
    }
  }
  return true;
}

static char _strLimbFile[MAX_PATH] = {0};

void Animator::AddNewSprite(void)
{
  if(_strLimbFile[0] == 0)
  {
    GetModuleFileName(GetModuleHandle(NULL),_strLimbFile,MAX_PATH);
    std::string strPath = GetPath(_strLimbFile) + "\\*.kv6";
    strcpy(_strLimbFile, strPath.c_str());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = _strLimbFile;
  ofn.nMaxFile = 260;
  ofn.lpstrFilter = TEXT("KV6 sprite (.kv6)\0*.kv6\0");
  ofn.lpstrDefExt = TEXT(".kv6");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
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
    if(_strnicmp(_strLimbFile, strPath.c_str(), strPath.size()) != 0)
    {
      MessageBox(NULL, "File is not under exe path!\n", "Error!", MB_OK);
      return;
    }
    const char *strRelative = _strLimbFile + strPath.size();
    if(!m_pFile->AddSprite(strRelative, m_pInstance))
    {
      MessageBox(NULL, "Adding limb failed!", "Error!", MB_OK);
    }
    m_Selected.resize(1);
    m_Selected[0] = m_pInstance->m_Sprites.back();
    m_bFileChanged = true;
  }
}

void Animator::ReplaceSprite(void)
{
  if(m_Selected.empty())
    return;

  if(_strLimbFile[0] == 0)
  {
    GetModuleFileName(GetModuleHandle(NULL),_strLimbFile,MAX_PATH);
    std::string strPath = GetPath(_strLimbFile) + "\\*.kv6";
    strcpy(_strLimbFile, strPath.c_str());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));
  ofn.lStructSize = sizeof(ofn);
  ofn.lpstrFile = _strLimbFile;
  ofn.nMaxFile = 260;
  ofn.lpstrFilter = TEXT("KV6 sprite (.kv6)\0*.kv6\0");
  ofn.lpstrDefExt = TEXT(".kv6");
  ofn.nFilterIndex = 1;
  ofn.lpstrFileTitle = NULL;
  ofn.nMaxFileTitle = 0;
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
    if(_strnicmp(_strLimbFile, strPath.c_str(), strPath.size()) != 0)
    {
      MessageBox(NULL, "File is not under exe path!\n", "Error!", MB_OK);
      return;
    }
    const char *strRelative = _strLimbFile + strPath.size();

    // find index for selected sprite
    unsigned int iIndex;
    if(!GetSpriteIndex(m_Selected[0], iIndex))
      return;

    // change filename
    m_pFile->m_SpriteFiles[iIndex] = strRelative;
    // change sprite
    delete m_Selected[0]->m_pSprite;
    m_Selected[0]->m_pSprite = new Sprite(strRelative);
    m_Selected[0]->UpdateSprite(Matrix44::mIdentity);

    m_bFileChanged = true;
  }
}

void Animator::DeleteSprite(void)
{
  for(unsigned int i = 0; i < m_Selected.size(); i++)
  {
    unsigned int iSprite = m_pFile->GetSprite(m_Selected[i]->GetSprite()->GetFilename());
    m_pFile->DeleteSprite(iSprite, m_pInstance);
  }
  m_Selected.clear();
}

void Animator::ChangeSpritePivot(void)
{
  if(m_Selected.empty())
    return;

  Sprite *pSprite = m_Selected[0]->GetSprite();
  kv6data *kv = pSprite->GetData()->voxnum;
  if(kv == NULL)
    return;

  Vector3D vPivot = Vector3D( kv->xpiv, kv->ypiv, kv->zpiv );
  Vector3D vOldPivot = vPivot;
  char strPivot[256];
  sprintf(strPivot, "%g,%g,%g", vPivot.x, vPivot.y, vPivot.z);

  StringInputDialog sid;
  sid.m_strText = strPivot;
  if(sid.Run("Edit pivot"))
  {
    sscanf(sid.m_strText.c_str(),"%g,%g,%g", &vPivot.x, &vPivot.y, &vPivot.z);

    Vector3D vPivotVXL = vPivot;
    kv->xpiv = vPivotVXL.x;
    kv->ypiv = vPivotVXL.y;
    kv->zpiv = vPivotVXL.z;

    Vector3D vDelta;
    vDelta.x = vPivot.x - vOldPivot.x;
    vDelta.y = vOldPivot.y - vPivot.y;
    vDelta.z = vPivot.z - vOldPivot.z;
    Matrix44 mDelta = TranslationMatrix(vDelta);

    // find all sprites using the same file
    for(unsigned int iIndex = 0; iIndex < m_pInstance->m_Sprites.size(); iIndex++)
    {
      if(m_pInstance->m_Sprites[iIndex]->GetSprite()->GetData()->voxnum != kv)
        continue;

      SpritePoser *pPoser = m_pInstance->m_Sprites[iIndex];
      Sprite *pSprite = pPoser->GetSprite();

      // modify current pose
      Matrix44 mSpriteTransform = Multiply(mDelta, pSprite->GetTransform());
      pPoser->m_qRotation = QuaternionFromMatrix(mSpriteTransform);
      pPoser->m_vPosition = mSpriteTransform.GetTranslation();

      Matrix44 mSpriteTransformInv = Inverse(mSpriteTransform);

      // modify all animations to match
      for(unsigned int iAnim = 0; iAnim < m_pFile->m_Animations.size(); iAnim++)
      {
        AnimationFile::Animation &anim = m_pFile->m_Animations[iAnim];
        for(unsigned int iFrame = 0; iFrame < anim.Frames.size(); iFrame++)
        {
          AnimationFile::Animation::Frame &frame = anim.Frames[iFrame];

          Matrix44 mTransform = MatrixFromQuaternion(frame.Poses[iIndex].qRotation);
          mTransform.SetTranslation(frame.Poses[iIndex].vPosition);
          mTransform = Multiply(mDelta, mTransform);
          
          frame.Poses[iIndex].qRotation = QuaternionFromMatrix(mTransform);
          frame.Poses[iIndex].vPosition = mTransform.GetTranslation();
        }
      }

      // modify all particles to match
      unsigned int iSpritePhysics = m_pFile->GetSpritePhysicsIndex(iIndex);
      if(iSpritePhysics != -1)
      {
        AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[iSpritePhysics];
        for(unsigned int i = 0; i < phys.Particles.size(); i++)
        {
          Vector3D vAbs = Transform(pSprite->GetTransform(), phys.Particles[i].vPosition);
          phys.Particles[i].vPosition = Transform(mSpriteTransformInv, vAbs);
        }
      }

      pPoser->UpdateSprite(Matrix44::mIdentity);
    }

    m_bFileChanged = true;
    m_SaveSprites.push_back( pSprite->GetFilename() );
  }
}

static std::vector<AnimationFile::SpritePose> _ClipboardPoses;

void Animator::CopyPoses(void)
{
  _ClipboardPoses.resize(m_Selected.size());
  for(unsigned int i = 0; i < m_Selected.size(); i++)
  {
    _ClipboardPoses[i].vPosition = m_Selected[i]->m_vPosition;
    _ClipboardPoses[i].qRotation = m_Selected[i]->m_qRotation;
    _ClipboardPoses[i].vScale = m_Selected[i]->m_vScale;
  }
}

void Animator::PastePoses(void)
{
  for(unsigned int i = 0; i < _ClipboardPoses.size() && i < m_Selected.size(); i++)
  {
    m_Selected[i]->m_vPosition = _ClipboardPoses[i].vPosition;
    m_Selected[i]->m_qRotation = _ClipboardPoses[i].qRotation;
    m_Selected[i]->m_vScale = _ClipboardPoses[i].vScale;
  }
  // add frame
  if(GetAnimator()->GetAutoSetFrame())
    GetAnimator()->GetAnimWindow()->OnAddFrame();
}

void Animator::ApplyRelativePosesToAll(void)
{
  if(m_Selected.size() != 2)
  {
    printf("Must select two sprites!\n");
    return;
  }

  if((unsigned int)m_pInstance->m_iCurrentAnim < m_pFile->m_Animations.size())
  {
    AnimationFile::Animation &anim = m_pFile->m_Animations[m_pInstance->m_iCurrentAnim];
    std::vector<AnimationFile::Animation::Frame> &frames = anim.Frames;

    // find index for selected sprites
    std::vector<unsigned int> Indices;
    for(unsigned int i = 0; i < m_Selected.size(); i++)
    {
      for(unsigned int j = 0; j < m_pInstance->m_Sprites.size(); j++)
      {
        if(m_Selected[i] == m_pInstance->m_Sprites[j])
        {
          Indices.push_back(j);
          break;
        }
      }
    }

    // calculate relative pose
    int iChildIndex = Indices[0];
    int iParentIndex = Indices[1];
    SpritePoser *pChild = m_Selected[0];
    SpritePoser *pParent = m_Selected[1];

    Matrix44 mChild = MatrixFromQuaternion(pChild->m_qRotation);
    mChild.SetTranslation(pChild->m_vPosition);

    Matrix44 mParent = MatrixFromQuaternion(pParent->m_qRotation);
    mParent.SetTranslation(pParent->m_vPosition);

    Matrix44 mInvParent = Inverse(mParent);
    Matrix44 mRelative = Multiply(mChild, mInvParent);

    for(unsigned int i = 0; i < frames.size(); i++)
    {
      // get parent in this frame
      Matrix44 mParentNow = MatrixFromQuaternion(anim.Frames[i].Poses[iParentIndex].qRotation);
      mParentNow.SetTranslation(anim.Frames[i].Poses[iParentIndex].vPosition);
      // apply relative transform
      Matrix44 mChildNow = Multiply(mRelative, mParentNow);

      // set new child pose
      anim.Frames[i].Poses[iChildIndex].qRotation = QuaternionFromMatrix(mChildNow);
      anim.Frames[i].Poses[iChildIndex].vPosition = mChildNow.GetTranslation();
      anim.Frames[i].Poses[iChildIndex].vScale = pChild->m_vScale;
    }
  }
}

void Animator::ApplyPosesToAll(void)
{
  if((unsigned int)m_pInstance->m_iCurrentAnim < m_pFile->m_Animations.size())
  {
    AnimationFile::Animation &anim = m_pFile->m_Animations[m_pInstance->m_iCurrentAnim];
    std::vector<AnimationFile::Animation::Frame> &frames = anim.Frames;

    // find indices for selected sprites
    std::vector<unsigned int> Indices;
    for(unsigned int i = 0; i < m_Selected.size(); i++)
    {
      for(unsigned int j = 0; j < m_pInstance->m_Sprites.size(); j++)
      {
        if(m_Selected[i] == m_pInstance->m_Sprites[j])
        {
          Indices.push_back(j);
          break;
        }
      }
    }

    for(unsigned int i = 0; i < frames.size(); i++)
      m_pFile->SetPoses(m_pInstance->m_iCurrentAnim, i, Indices, m_Selected);
  }
}

void Animator::ResetCamera(void)
{
  m_pCameraTool->Reset();
}

void Animator::SetFPSView(void)
{
  m_pCameraTool->SetFPSView();
}

void Animator::ToggleGrid(void)
{
  if(m_bRenderGrid)
  {
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_VIEW_GRID,MF_UNCHECKED);
    m_bRenderGrid = false;
  }
  else
  {
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_VIEW_GRID,MF_CHECKED);
    m_bRenderGrid = true;
  }
}

void Animator::EditPosition(void)
{
  std::vector<SpritePoser *> &Selected = m_bPhysicsMode ? m_SelectedParticles : m_Selected;

  if(Selected.empty())
    return;

  char strName[256];
  sprintf(strName, "%g,%g,%g", Selected[0]->m_vPosition.x, Selected[0]->m_vPosition.y, Selected[0]->m_vPosition.z);
  StringInputDialog sid;
  sid.m_strText = strName;
  if(sid.Run("Edit position"))
  {
    sscanf(sid.m_strText.c_str(),"%g,%g,%g", &Selected[0]->m_vPosition.x, &Selected[0]->m_vPosition.y, &Selected[0]->m_vPosition.z);
    if(GetAnimator()->GetAutoSetFrame())
      GetAnimator()->GetAnimWindow()->OnAddFrame();
    GetAnimator()->FileChanged();
  }
}

void Animator::EditRotation(void)
{
  if(m_Selected.empty())
    return;

  char strName[256];
  Vector3D vRot = RadiansToDegrees(GetEuler(MatrixFromQuaternion(m_Selected[0]->m_qRotation)));
  if(vRot.x == -0.0f)
    vRot.x = 0.0f;
  if(vRot.y == -0.0f)
    vRot.y = 0.0f;
  if(vRot.z == -0.0f)
    vRot.z = 0.0f;
  sprintf(strName, "%g,%g,%g", vRot.x, vRot.y, vRot.z);
  StringInputDialog sid;
  sid.m_strText = strName;
  if(sid.Run("Edit rotation"))
  {
    sscanf(sid.m_strText.c_str(),"%g,%g,%g", &vRot.x, &vRot.y, &vRot.z);
    m_Selected[0]->m_qRotation = QuaternionFromEuler(DegreesToRadians(vRot));

    if(GetAnimator()->GetAutoSetFrame())
      GetAnimator()->GetAnimWindow()->OnAddFrame();
    GetAnimator()->FileChanged();
  }
}

void Animator::EditScale(void)
{
  std::vector<SpritePoser *> &Selected = m_bPhysicsMode ? m_SelectedParticles : m_Selected;

  if(Selected.empty())
    return;

  char strName[256];
  sprintf(strName, "%g,%g,%g", Selected[0]->m_vScale.x, Selected[0]->m_vScale.y, Selected[0]->m_vScale.z);
  StringInputDialog sid;
  sid.m_strText = strName;
  if(sid.Run("Edit scale"))
  {
    sscanf(sid.m_strText.c_str(),"%g,%g,%g", &Selected[0]->m_vScale.x, &Selected[0]->m_vScale.y, &Selected[0]->m_vScale.z);
    if(GetAnimator()->GetAutoSetFrame())
      GetAnimator()->GetAnimWindow()->OnAddFrame();
    GetAnimator()->FileChanged();
  }
}

void Animator::AddParticle(void)
{
  m_SelectedParticles.clear();
  // go to sprite picking mode
  m_Selected.clear();
  m_iSpritePicking = SpritePicking_AddParticle;
}

void Animator::AddParticle_internal(void)
{
  m_iSpritePicking = 0;

  if(m_Selected.empty())
    return;

  SpritePoser *pSprite = m_Selected[0];

  // find sprite index
  unsigned int iIndex;
  if(!GetSpriteIndex(pSprite, iIndex))
    return;

  unsigned int iSpritePhysics = m_pFile->AddSpritePhysics(iIndex);
  AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[iSpritePhysics];
  AnimationFile::PhysicsParticle part;
  part.vPosition = Transform( Inverse(pSprite->GetSprite()->GetTransform()), ((SelectionTool *)m_Tools[0])->m_vSelectHit );
  part.fRadius = 10.0f;

  unsigned int iParticle = phys.Particles.size();
  phys.Particles.push_back(part);

  CreateParticlePosers();

  GetAnimator()->FileChanged();

  m_SelectedParticles.resize(1);
  m_SelectedParticles[0] = GetParticlePoser(iSpritePhysics, iParticle);
}

void Animator::ToggleParticleCollision(bool bEnabled)
{
  if(m_SelectedParticles.empty())
  {
    return;
  }

  std::vector<ParticlePoserLink> SelectedParticles;
  for(unsigned int i = 0; i < m_SelectedParticles.size(); i++)
  {
    unsigned int iLinkIndex = GetParticleLinkIndex(m_SelectedParticles[i]);
    ParticlePoserLink &link = m_ParticlePoserLinks[iLinkIndex];
    AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[link.iSpritePhysics];
    AnimationFile::PhysicsParticle &particle = phys.Particles[link.iParticle];
    if(bEnabled)
      particle.fRadius = fabsf(particle.fRadius);
    else
      particle.fRadius = -fabsf(particle.fRadius);
    SelectedParticles.push_back(link);
  }

  CreateParticlePosers();

  for(unsigned int i = 0; i < SelectedParticles.size(); i++)
  {
    m_SelectedParticles.push_back(GetParticlePoser(SelectedParticles[i].iSpritePhysics, SelectedParticles[i].iParticle));
  }

  GetAnimator()->FileChanged();
}

void Animator::RemoveParticle(void)
{
  if(m_SelectedParticles.empty())
  {
    return;
  }

  for(unsigned int i = 0; i < m_SelectedParticles.size(); i++)
  {
    unsigned int iLinkIndex = GetParticleLinkIndex(m_SelectedParticles[i]);
    ParticlePoserLink &link = m_ParticlePoserLinks[iLinkIndex];

    unsigned int iSpritePhysics = link.iSpritePhysics;
    unsigned int iParticle = link.iParticle;
    m_pFile->RemoveParticle(iSpritePhysics, iParticle);

    // fix particle links
    for(unsigned int j = i + 1; j < m_ParticlePoserLinks.size(); j++)
    {
      if(m_ParticlePoserLinks[j].iSpritePhysics == iSpritePhysics
        && m_ParticlePoserLinks[j].iParticle > iParticle)
      {
        m_ParticlePoserLinks[j].iParticle--;
      }
    }
  }

  m_pFile->RemoveEmptySpritePhysics();
  CreateParticlePosers();
  GetAnimator()->FileChanged();
}

void Animator::ConnectParticles(bool bConnect)
{
  if(m_SelectedParticles.size() < 2)
  {
    printf("Must select at least two particles to connect!\n");
    return;
  }

  GetAnimator()->FileChanged();

  for(unsigned int s1 = 0; s1 < m_SelectedParticles.size(); s1++)
  for(unsigned int s2 = s1 + 1; s2 < m_SelectedParticles.size(); s2++)
  {
    AnimationFile::PhysicsConstraint constraint;
    constraint.iSpritePhysics1 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s1])].iSpritePhysics;
    constraint.iParticle1 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s1])].iParticle;
    constraint.iSpritePhysics2 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s2])].iSpritePhysics;
    constraint.iParticle2 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s2])].iParticle;
    constraint.fDistance = 0.0f;

    // find match
    bool bAlreadyConnected = false;
    for(unsigned int i = 0; i < m_pFile->m_PhysicsConstraints.size(); i++)
    {
      AnimationFile::PhysicsConstraint &const2 = m_pFile->m_PhysicsConstraints[i];
      if(const2.ConnectedToSame(constraint))
      {
        // remove
        if(!bConnect)
        {
          m_pFile->m_PhysicsConstraints[i] = m_pFile->m_PhysicsConstraints.back();
          m_pFile->m_PhysicsConstraints.pop_back();
        }
        else
        {
          bAlreadyConnected = true;
        }
        break;
      }
    }

    // add new
    if(bConnect && !bAlreadyConnected)
    {
      m_pFile->m_PhysicsConstraints.push_back(constraint);
    }
  }
}


void Animator::SetConnectDistance(void)
{
  if(m_SelectedParticles.size() < 2)
  {
    printf("Must select at least two particles to connect!\n");
    return;
  }

  // make sure they are connected
  ConnectParticles(true);

  // collect pointers to constraint distances
  std::vector<float *> Distances;
  for(unsigned int s1 = 0; s1 < m_SelectedParticles.size(); s1++)
  for(unsigned int s2 = s1 + 1; s2 < m_SelectedParticles.size(); s2++)
  {
    AnimationFile::PhysicsConstraint constraint;
    constraint.iSpritePhysics1 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s1])].iSpritePhysics;
    constraint.iParticle1 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s1])].iParticle;
    constraint.iSpritePhysics2 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s2])].iSpritePhysics;
    constraint.iParticle2 = m_ParticlePoserLinks[GetParticleLinkIndex(m_SelectedParticles[s2])].iParticle;

    // find match
    for(unsigned int i = 0; i < m_pFile->m_PhysicsConstraints.size(); i++)
    {
      AnimationFile::PhysicsConstraint &const2 = m_pFile->m_PhysicsConstraints[i];
      if(const2.ConnectedToSame(constraint))
      {
        Distances.push_back(&const2.fDistance);
        break;
      }
    }
  }

  // this should never happen, but hey.
  if(Distances.empty())
    return;

  bool bDistanceMismatch = false;
  float fCurrentDistance = *Distances[0];
  for(unsigned int i = 0; i < Distances.size(); i++)
  {
    if(*Distances[i] != fCurrentDistance)
    {
      bDistanceMismatch = true;
      break;
    }
  }

  char strName[256];
  if(bDistanceMismatch)
    strName[0] = 0;
  else
    sprintf(strName, "%g", fabsf(fCurrentDistance));

  StringInputDialog sid;
  sid.m_strText = strName;
  if(sid.Run("Edit connect distance"))
  {
    sscanf(sid.m_strText.c_str(),"%g", &fCurrentDistance);

    if(fCurrentDistance >= 0)
    {
      for(unsigned int i = 0; i < Distances.size(); i++)
      {
        *Distances[i] = -fCurrentDistance;
      }
    }
    GetAnimator()->FileChanged();
  }
}

void Animator::SetParticleParent(void)
{
  if(m_SelectedParticles.empty())
  {
    return;
  }

  GetAnimator()->FileChanged();

  // go to sprite picking mode
  m_Selected.clear();
  m_iSpritePicking = SpritePicking_SetParticleParent;
}

void Animator::SetParticleParent_internal(void)
{
  m_iSpritePicking = 0;
  if(m_Selected.empty())
    return;

  // find sprite index
  unsigned int iIndex;
  if(!GetSpriteIndex(m_Selected[0], iIndex))
    return;

  unsigned int iNewSpritePhysics = m_pFile->AddSpritePhysics(iIndex);

  std::vector<ParticlePoserLink> SelectedParticles;
  for(unsigned int i = 0; i < m_SelectedParticles.size(); i++)
  {
    unsigned int iLinkIndex = GetParticleLinkIndex(m_SelectedParticles[i]);
    ParticlePoserLink &link = m_ParticlePoserLinks[iLinkIndex];

    AnimationFile::PhysicsParticle &particle = m_pFile->m_SpritePhysics[ link.iSpritePhysics ].Particles[ link.iParticle ];

    Sprite *pOldSprite = m_pInstance->m_Sprites[ m_pFile->m_SpritePhysics[link.iSpritePhysics].iSprite ]->GetSprite();
    Sprite *pNewSprite = m_Selected[0]->GetSprite();

    // calculate new relative position
    Vector3D vPosAbs = Transform(pOldSprite->GetTransform(), particle.vPosition);
    particle.vPosition = Transform(Inverse(pNewSprite->GetTransform()), vPosAbs);

    // change spritephysics
    unsigned int iNewParticle = m_pFile->ChangeParticleSpritePhysics(link.iSpritePhysics, link.iParticle, iNewSpritePhysics);
  }

  m_pFile->RemoveEmptySpritePhysics();
  CreateParticlePosers();
  GetAnimator()->FileChanged();
}

void Animator::RemoveMovePointsFromSprite(void)
{
  m_Selected.clear();
  m_iSpritePicking = SpritePicking_RemoveMovePointsFromSprite;
  return;
}

void Animator::RemoveMovePointsFromSprite_internal(void)
{
  m_iSpritePicking = 0;
  if(m_Selected.empty())
    return;

  // find sprite index
  unsigned int iIndex;
  if(!GetSpriteIndex(m_Selected[0], iIndex))
    return;

  unsigned int iSpritePhysics = m_pFile->GetSpritePhysicsIndex(iIndex);
  if(iSpritePhysics == -1)
    return;

  AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[iSpritePhysics];
  for(int j = 0; j < 3; j++)
    phys.MovePointParticles[j] = AnimationFile::ParticleIndex();

  m_pFile->RemoveEmptySpritePhysics();
  CreateParticlePosers();
  GetAnimator()->FileChanged();
}

void Animator::SetParticleAsMovePoint(bool bSet)
{
  if(m_SelectedParticles.empty())
  {
    return;
  }

  GetAnimator()->FileChanged();

  if(bSet)
  {
    // go to sprite picking mode
    m_Selected.clear();
    m_iSpritePicking = SpritePicking_SetParticleAsMovePoint;
    return;
  }

  std::vector<ParticlePoserLink> SelectedParticles;
  for(unsigned int i = 0; i < m_SelectedParticles.size(); i++)
  {
    unsigned int iLinkIndex = GetParticleLinkIndex(m_SelectedParticles[i]);
    ParticlePoserLink &link = m_ParticlePoserLinks[iLinkIndex];

    // loop through all physics
    for(unsigned int p = 0; p < m_pFile->m_SpritePhysics.size(); p++)
    {
      AnimationFile::SpritePhysics &phys = m_pFile->m_SpritePhysics[p];

      // find match
      for(int j = 0; j < 3; j++)
      {
        if(phys.MovePointParticles[j] == AnimationFile::ParticleIndex(link.iSpritePhysics, link.iParticle))
        {
          phys.MovePointParticles[j].iParticle = -1;
          phys.MovePointParticles[j].iSpritePhysics = -1;
        }
      }
    }
  }

}

void Animator::SetParticleAsMovePoint_internal(void)
{
  m_iSpritePicking = 0;
  if(m_Selected.empty())
    return;

  // find sprite index
  unsigned int iIndex;
  if(!GetSpriteIndex(m_Selected[0], iIndex))
    return;

  // get physics for selected sprite
  AnimationFile::SpritePhysics &selected_phys = m_pFile->m_SpritePhysics[m_pFile->AddSpritePhysics(iIndex)];

  std::vector<ParticlePoserLink> SelectedParticles;
  for(unsigned int i = 0; i < m_SelectedParticles.size(); i++)
  {
    unsigned int iLinkIndex = GetParticleLinkIndex(m_SelectedParticles[i]);
    ParticlePoserLink &link = m_ParticlePoserLinks[iLinkIndex];

    // check that doesn't already exist
    if(selected_phys.MovePointParticles[0] == AnimationFile::ParticleIndex(link.iSpritePhysics, link.iParticle))
      continue;
    if(selected_phys.MovePointParticles[1] == AnimationFile::ParticleIndex(link.iSpritePhysics, link.iParticle))
      continue;
    if(selected_phys.MovePointParticles[2] == AnimationFile::ParticleIndex(link.iSpritePhysics, link.iParticle))
      continue;

    // find free element
    int j = 0;
    for(; j < 2; j++)
    {
      if(!selected_phys.MovePointParticles[j].IsSet())
        break;
    }
    selected_phys.MovePointParticles[j].iParticle = link.iParticle;
    selected_phys.MovePointParticles[j].iSpritePhysics = link.iSpritePhysics;
  }

  m_Selected.clear();
}

void Animator::TogglePhysicsMode(void)
{
  if(m_bPhysicsMode)
  {
    EnableWindow(m_pAnimWindow->m_hWindow, TRUE);
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_MODE,MF_UNCHECKED);

    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_COPYPOSES, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_PASTEPOSES, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_APPLYPOSETOALL, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_ROTATION, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_PARENT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_ADD, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_REPLACE, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_DEL, MF_ENABLED);

    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_COPY, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_PASTE, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_ADDPARTICLE, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_DELETEPARTICLE, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_ENABLECOLLISION, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_DISABLECOLLISION, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETCONNECTDISTANCE, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_CONNECT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_UNCONNECT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETPARENT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETMOVEPOINT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_UNSETMOVEPOINT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_REMMOVEPOINT, MF_GRAYED);

    m_bPhysicsMode = false;
    m_iSpritePicking = 0;
    DestroyParticlePosers();
  }
  else
  {
    EnableWindow(m_pAnimWindow->m_hWindow, FALSE);
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_MODE,MF_CHECKED);

    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_COPYPOSES, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_PASTEPOSES, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_APPLYPOSETOALL, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_ROTATION, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_EDIT_PARENT, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_ADD, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_REPLACE, MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_LIMBS_DEL, MF_GRAYED);

    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_COPY, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_PASTE, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_ADDPARTICLE, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_DELETEPARTICLE, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_ENABLECOLLISION, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_DISABLECOLLISION, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETCONNECTDISTANCE, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_CONNECT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_UNCONNECT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETPARENT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_SETMOVEPOINT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_UNSETMOVEPOINT, MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_REMMOVEPOINT, MF_ENABLED);

    m_bPhysicsMode = true;
    CreateParticlePosers();
    m_Selected.clear();
  }
}

unsigned int Animator::GetParticleLinkIndex(SpritePoser *pPoser)
{
  for(unsigned int i = 0; i < m_ParticlePosers.size(); i++)
  {
    if(pPoser == m_ParticlePosers[i])
      return i;
  }
  printf("WARNING: No particle link for poser found!\n");
  return -1;
}

SpritePoser *Animator::GetParticlePoser(unsigned int iSpritePhysics, unsigned int iParticle)
{
  for(unsigned int i = 0; i < m_ParticlePoserLinks.size(); i++)
  {
    if(m_ParticlePoserLinks[i].iParticle == iParticle && m_ParticlePoserLinks[i].iSpritePhysics == iSpritePhysics)
      return m_ParticlePosers[i];
  }
  printf("WARNING: No particle poser for link found!\n");
  return NULL;
}

void Animator::CreateParticlePosers(void)
{
  DestroyParticlePosers();

  std::vector<AnimationFile::SpritePhysics> &SpritePhysics = m_pFile->m_SpritePhysics;
  for(unsigned int i = 0; i < SpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = SpritePhysics[i];     
    for(unsigned int p = 0; p < phys.Particles.size(); p++)
    {
      AnimationFile::PhysicsParticle &particle = phys.Particles[p];
      Vector3D vPos = Transform(m_pInstance->m_Sprites[phys.iSprite]->GetSprite()->GetTransform(), particle.vPosition);

      // create poser
      if(particle.fRadius < 0.0f)
        m_ParticlePosers.push_back(new SpritePoser(new Sprite("kv6/particleposer_nocollision.kv6")));
      else
        m_ParticlePosers.push_back(new SpritePoser(new Sprite("kv6/particleposer.kv6")));
      m_ParticlePosers.back()->m_vScale = Vector3D(fabsf(particle.fRadius) / fParticlePoserSpriteRadius, fabsf(particle.fRadius) / fParticlePoserSpriteRadius, fabsf(particle.fRadius) / fParticlePoserSpriteRadius);
      m_ParticlePosers.back()->m_vPosition = vPos;
      m_ParticlePosers.back()->UpdateSprite(Matrix44::mIdentity);

      // create link
      ParticlePoserLink link;
      link.iParticle = p;
      link.iSpritePhysics = i;
      m_ParticlePoserLinks.push_back(link);
    }
  }
}

void Animator::DestroyParticlePosers(void)
{
  for(unsigned int i = 0; i < m_ParticlePosers.size(); i++)
    delete m_ParticlePosers[i];
  m_ParticlePosers.clear();
  m_ParticlePoserLinks.clear();
  m_SelectedParticles.clear();
}

void Animator::TogglePhysicsPreviewMode(void)
{
  if(m_pInstance->IsRagdoll())
  {
    EnableWindow(m_pAnimWindow->m_hWindow, TRUE);
    EnableMenuItem(GetMenu(ghwnd),0,MF_BYPOSITION|MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),1,MF_BYPOSITION|MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),2,MF_BYPOSITION|MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),3,MF_BYPOSITION|MF_ENABLED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_MODE, MF_ENABLED);
    m_pInstance->DestroyRagdoll();
    DrawMenuBar(ghwnd);
  }
  else
  {
    if(m_bPhysicsMode)
      TogglePhysicsMode();

    m_pInstance->CreateRagdoll();
    if(!m_pInstance->IsRagdoll())
      return;

    EnableWindow(m_pAnimWindow->m_hWindow, FALSE);
    EnableMenuItem(GetMenu(ghwnd),0,MF_BYPOSITION|MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),1,MF_BYPOSITION|MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),2,MF_BYPOSITION|MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),3,MF_BYPOSITION|MF_GRAYED);
    EnableMenuItem(GetMenu(ghwnd),ID_MAINMENU_RAGDOLL_MODE, MF_GRAYED);
    
    DrawMenuBar(ghwnd);
  }
}

static std::vector<AnimationFile::PhysicsConstraint> _CopiedConstraints;
static std::vector<AnimationFile::SpritePhysics> _CopiedSpritePhysics;
void Animator::CopyPhysics(void)
{
  _CopiedSpritePhysics = m_pFile->m_SpritePhysics;
  _CopiedConstraints = m_pFile->m_PhysicsConstraints;

  // calculate absolute positions
  for(unsigned int i = 0; i < _CopiedSpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = _CopiedSpritePhysics[i];
    Sprite *pSprite = m_pInstance->m_Sprites[ phys.iSprite ]->GetSprite();
    for(unsigned int j = 0; j < phys.Particles.size(); j++)
    {
      phys.Particles[j].vPosition = Transform(pSprite->GetTransform(), phys.Particles[j].vPosition);
    }
  }
}

void Animator::PastePhysics(void)
{
  if(m_pFile->m_SpriteFiles.empty())
    return;

  if(_CopiedSpritePhysics.empty() && _CopiedConstraints.empty())
    return;

  unsigned int iNewSpritePhysics = m_pFile->AddSpritePhysics(0);
  AnimationFile::SpritePhysics &phys_new = m_pFile->m_SpritePhysics[ iNewSpritePhysics ];

  // add particles
  std::vector<unsigned int> ParticleOffsets(_CopiedSpritePhysics.size());
  for(unsigned int i = 0; i < _CopiedSpritePhysics.size(); i++)
  {
    AnimationFile::SpritePhysics &phys = _CopiedSpritePhysics[i];
    // reset move points
    phys.MovePointParticles[0] = phys.MovePointParticles[1] = phys.MovePointParticles[2] = AnimationFile::ParticleIndex();

    ParticleOffsets[i] = phys_new.Particles.size();
    phys_new.Particles.insert(phys_new.Particles.end(), phys.Particles.begin(), phys.Particles.end());
  }

  // add constraints
  unsigned int iConstraintOffset = m_pFile->m_PhysicsConstraints.size();
  m_pFile->m_PhysicsConstraints.resize(iConstraintOffset + _CopiedConstraints.size());
  for(unsigned int i = 0; i < _CopiedConstraints.size(); i++)
  {
    AnimationFile::PhysicsConstraint &constraint = _CopiedConstraints[i];
    AnimationFile::PhysicsConstraint &new_constraint = m_pFile->m_PhysicsConstraints[iConstraintOffset + i];
    new_constraint.iSpritePhysics1 = iNewSpritePhysics;
    new_constraint.iSpritePhysics2 = iNewSpritePhysics;
    new_constraint.iParticle1 = ParticleOffsets[constraint.iSpritePhysics1] + constraint.iParticle1;
    new_constraint.iParticle2 = ParticleOffsets[constraint.iSpritePhysics2] + constraint.iParticle2;
    new_constraint.fDistance = constraint.fDistance;
  }

  CreateParticlePosers();
}

bool Animator::GetSpriteIndex(SpritePoser *pSprite, unsigned int &iIndex)
{
  // find sprite index
  for(unsigned int i = 0; i < m_pInstance->m_Sprites.size(); i++)
  {
    if(pSprite == m_pInstance->m_Sprites[i])
    {
      iIndex = i;
      return true;
    }
  }
  return false;
}
