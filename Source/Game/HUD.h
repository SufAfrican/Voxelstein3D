#include "Game/Entities/EntityTree.h"
#include "Game/Entities/PlayerEntity.h"
#include "Game/Weapons/Weapon.h"

#ifdef LoadImage
  #undef LoadImage
#endif

inline void DrawRect2D(float fStartX, float fStartY, float fEndX, float fEndY, int iColor)
{
  drawline2d(fStartX, fStartY, fEndX, fStartY, iColor);
  drawline2d(fEndX, fStartY, fEndX, fEndY, iColor);
  drawline2d(fEndX, fEndY, fStartX, fEndY, iColor);
  drawline2d(fStartX, fEndY, fStartX, fStartY, iColor);
}

static const float fNodeDrawOffsetX = 300.0f;
static const float fNodeDrawOffsetY = 200.0f;
static const float fNodeDrawScale = 0.25f;

inline void DrawNode(EntityTree::QuadTreeNode *pNode)
{
  DrawRect2D(fNodeDrawOffsetX + pNode->m_vCorner.x * fNodeDrawScale,
             fNodeDrawOffsetY + pNode->m_vCorner.y * fNodeDrawScale,
             fNodeDrawOffsetX + (pNode->m_vCorner.x + pNode->m_fSize) * fNodeDrawScale,
             fNodeDrawOffsetY + (pNode->m_vCorner.y + pNode->m_fSize) * fNodeDrawScale, 0xFF008000);
  print4x6(fNodeDrawOffsetX + pNode->m_vCorner.x * fNodeDrawScale,
           fNodeDrawOffsetY + pNode->m_vCorner.y * fNodeDrawScale,0xFFFFFF,-1,"%i",pNode->m_Entities.size());
}

void DrawHUD(float fDeltaTime)
{
  int iScreenX, iScreenY;
  getscreensize(iScreenX, iScreenY);

  float fTime = Time::GetTime();

  PlayerEntity *pPlayer = GetGame()->GetLocalPlayer();

  // draw weapon info
  Weapon *pWeapon = pPlayer->GetSelectedWeapon();
  if(pWeapon != NULL)
  {
    VoxlapImage *pImg = pWeapon->GetHUDImage();
    if(pImg != NULL)
    {
      int x = iScreenX - 40;
      int y = iScreenY - 30;
      drawtile(pImg->pData, pImg->iBytesPerLine, pImg->iWidth, pImg->iHeight, pImg->iWidth<<15, pImg->iHeight<<15,
               ((long)x)<<16, ((long)y)<<16, 65536, 65536, 0, -1);
    }

    if(pWeapon->GetAmmo() >= 0)
      print6x8(iScreenX - 60, iScreenY - 10,0xF0F0c0,-1,"Ammo:%i",pWeapon->GetAmmo());
  }

  // draw health
  {
    const char *strFrame;

    int iOffsetX = 10;
    int iOffsetY = 105;

    if(pPlayer->m_iLastFoundChaingun != 0 && GetGame()->GetCurrentTick() - pPlayer->m_iLastFoundChaingun < 3 * Game::iTicksPerSecond)
    {
      strFrame = "chaingun_pickup.PNG";
    }
    else if(pPlayer->m_iLastFoundTreasure != 0 && GetGame()->GetCurrentTick() - pPlayer->m_iLastFoundTreasure < 1 * Game::iTicksPerSecond)
    {
      if(GetGame()->GetCurrentTick() - pPlayer->m_iLastFoundTreasure < Game::iTicksPerSecond/4)
        strFrame = "oh_treasure_pickup1.PNG";
      else
        strFrame = "oh_treasure_pickup2.PNG";
    }
    else if(pPlayer->m_iHoldingTriggerTime > 1 * Game::iTicksPerSecond)
    {
      strFrame = "look_center2.PNG";
    }
    else
    {
      float fLookFrame = sinf(fTime);
      if(fLookFrame < -0.5f)
        strFrame = "look_right.PNG";
      else if(fLookFrame > 0.5f)
        strFrame = "look_left.PNG";
      else
        strFrame = "look_center.PNG";
    }

    const char *strStatus;
    if(pPlayer->m_iHealth <= 0)
    {
      strStatus = "0";
      strFrame = "dead.PNG";
    }
    else if(pPlayer->m_iHealth <= 25)
    {
      strStatus = "25";
      iOffsetX -= 2;
      iOffsetY += 4;
    }
    else if(pPlayer->m_iHealth <= 50)
    {
      strStatus = "50";
      iOffsetX -= 2;
      iOffsetY += 4;
    }
    else if(pPlayer->m_iHealth <= 75)
    {
      strStatus = "75";
      iOffsetX -= 2;
      iOffsetY += 4;
    }
    else
      strStatus = "normal";

    char strFile[128];
    sprintf(strFile, "png/health_hud_png/%s/%s", strStatus, strFrame);
    VoxlapImage *pImg = Voxlap::LoadImage(strFile);
    drawtile(pImg->pData, pImg->iBytesPerLine, pImg->iWidth, pImg->iHeight, iOffsetX, iOffsetY,
      ((long)iOffsetX*iScreenX/640)<<16, ((long)iScreenY - iOffsetY*iScreenY/480)<<16, 65536*iScreenX/640, 65536*iScreenY/480, 0, -1);
  }


  // draw entity tree
  if(Config::Debug_bVisualizeEntityTree)
  {
    EntityTree *pTree = GetGame()->GetEntityTree();
    std::vector<EntityTree::QuadTreeNode *> NodeStack;
    unsigned int iStackFront = 0;
    NodeStack.push_back(pTree->m_pRoot);
    while(iStackFront < NodeStack.size())
    {
      // pop node
      EntityTree::QuadTreeNode *pNode = NodeStack[iStackFront];
      iStackFront++;

      DrawNode(pNode);

      // add children to stack
      NodeStack.reserve(NodeStack.size() + 4);
      for(int i = 0; i < 4; i++)
      {
        if(pNode->m_pChildren[i] != NULL)
          NodeStack.push_back(pNode->m_pChildren[i]);
      }
    }
    /*DrawRect2D(fNodeDrawOffsetX + (pPlayer->m_vPosition.x - pPlayer->m_vSize.x * 0.5f) * fNodeDrawScale,
               fNodeDrawOffsetY + (pPlayer->m_vPosition.z - pPlayer->m_vSize.z * 0.5f) * fNodeDrawScale,
               fNodeDrawOffsetX + (pPlayer->m_vPosition.x + pPlayer->m_vSize.x * 0.5f) * fNodeDrawScale,
               fNodeDrawOffsetY + (pPlayer->m_vPosition.z + pPlayer->m_vSize.z * 0.5f) * fNodeDrawScale, 0xFFFF0000);*/
  }
}
