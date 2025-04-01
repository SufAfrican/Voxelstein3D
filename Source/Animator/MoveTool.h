#pragma once

/// Tool for moving sprites
class MoveTool : public AnimatorTool
{
public:
  bool m_bMoving;
  Vector3D m_vMovingPoint;
  Vector3D m_vMovingNormal;
  int m_iMovingLock;

public:
  MoveTool()
  {
    m_bMoving = false;
    m_iMovingLock = -1;
  }

  void Run(float fDeltaTime)
  {
    const std::vector<SpritePoser *> &Sprites = GetAnimator()->GetSelected();
    if(Sprites.empty())
    {
      m_bMoving = false;
      return;
    }

    Vector3D vCursorPos = GetAnimator()->GetCursorPosition();
    Vector3D vCursorDir = Normalize(vCursorPos - GetAnimator()->GetCameraMatrix().GetTranslation());

    Vector3D vCenterPos(0,0,0);
    for(unsigned int i = 0; i < Sprites.size(); i++)
    {
      vCenterPos += Sprites[i]->m_vPosition;
    }
    vCenterPos /= Sprites.size();

    float fArrowLength = 20.0f;
    float fArrowWidth = 4.0f;
    bool bShortcutDown = GetAnimator()->IsKeyDown(VK_CONTROL) && !GetAnimator()->IsKeyDown(VK_MENU) && !GetAnimator()->IsKeyDown(VK_SHIFT);
    int iSelectedAxis = -1;
    float fAxisHitDist = 0.0f;
    if(bShortcutDown && !m_bMoving)
    {
      // draw preview arrows
      for(int i = 0; i < 3; i++)
      {
        int iHitAxis = 0;
        Vector3D vBoxSize = Vector3D(fArrowWidth, fArrowWidth, fArrowWidth);
        vBoxSize[i] = fArrowLength;
        int iCol[3] = {0,0,0};
        iCol[i] = 255;
        Vector3D vLineDir(0,0,0); vLineDir[i] = 1;

        // select with mouse over
        if(iSelectedAxis < 0 && IntersectRayBox(vCursorPos, vCursorDir, vCenterPos + vBoxSize * 0.5f, 0.5f * vBoxSize, fAxisHitDist, iHitAxis))
        {
          iSelectedAxis = i;
          iCol[0] += 192;
          iCol[1] += 192;
          iCol[2] += 192;
          iCol[i] = 255;
        }
          
        DrawArrow(vCenterPos, vCenterPos + vLineDir * fArrowLength, 0.5f * fArrowWidth, GetAnimator()->GetCameraMatrix().GetFrontVector(), iCol[0], iCol[1], iCol[2], false);
      }
    }
    else if(bShortcutDown && m_bMoving && m_iMovingLock >= 0)
    {
      // draw axis currently moved along
      Vector3D vBoxSize = Vector3D(fArrowWidth, fArrowWidth, fArrowWidth);
      vBoxSize[m_iMovingLock] = fArrowLength;
      int iCol[3] = {0,0,0};
      iCol[m_iMovingLock] = 255;
      Vector3D vLineDir(0,0,0); vLineDir[m_iMovingLock] = 1;
      DrawArrow(vCenterPos, vCenterPos + vLineDir * fArrowLength, 0.5f * fArrowWidth, GetAnimator()->GetCameraMatrix().GetFrontVector(), iCol[0],iCol[1],iCol[2], false);
    }

    // holding ctrl + mouse
    if(bShortcutDown && GetAnimator()->IsMouseLeftDown())
    {
      // just started moving
      if(!m_bMoving)
      {
        // arrow axis
        if(iSelectedAxis >= 0)
        {
          m_vMovingPoint = vCursorPos + vCursorDir * fAxisHitDist;

          Vector3D vHitAxis(0,0,0);
          vHitAxis[iSelectedAxis] = 1;
          Vector3D vCamera = GetAnimator()->GetCameraMatrix().GetTranslation();
          m_vMovingNormal = Normalize(vCenterPos + vHitAxis * Dot(vHitAxis, vCamera - vCenterPos) - vCamera); 
          m_iMovingLock = iSelectedAxis;
          m_bMoving = true;
        }

        // ray to voxels
        if(!m_bMoving)
        {
          float fNearestVoxelHit = FLT_MAX;
          for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
          {
            SpritePoser *pSprite = Sprites[iSprite];

            Voxlap::HitInfo info;
            if(Voxlap::HitScan(pSprite->GetSprite(), vCursorPos, vCursorDir, 1000.0f, info))
            {
              if(info.fDistance < fNearestVoxelHit)
              {
                fNearestVoxelHit = info.fDistance;
                m_vMovingPoint = info.vPos;
                m_vMovingNormal = -GetAnimator()->GetCameraMatrix().GetFrontVector();
                m_iMovingLock = -1;
                m_bMoving = true;
              }
            }
          }
        }

        // ray to bounding box
        if(!m_bMoving)
        {
          float fNearestBoundingBoxHit = FLT_MAX;
          for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
          {
            SpritePoser *pSprite = Sprites[iSprite];

            Vector3D vMin, vMax;
            pSprite->GetSprite()->CalculateBoundingBox(vMin, vMax, true, true);
            float fHitDist = 0.0f;
            int iHitAxis = 0;
            if(IntersectRayBox(vCursorPos, vCursorDir, (vMin+vMax)*0.5f, (vMax-vMin)*0.5f, fHitDist, iHitAxis))
            {
              if(fHitDist < fNearestBoundingBoxHit)
              {
                fNearestBoundingBoxHit = fHitDist;
                m_vMovingPoint = vCursorPos + vCursorDir * fHitDist;
                m_vMovingNormal = -GetAnimator()->GetCameraMatrix().GetFrontVector();
                m_iMovingLock = -1;
                m_bMoving = true;
              }
            }
          }
        }
        return;
      }
      else
      {
        // intersect with plane
        float fDot = Dot(m_vMovingNormal, vCursorDir);
        float fPlaneD = -Dot(m_vMovingPoint, m_vMovingNormal);
        float fIntersectionDist = -(Dot(vCursorPos, m_vMovingNormal) + fPlaneD) / fDot;
        Vector3D vIntersection = vCursorPos + vCursorDir * fIntersectionDist;

        // just move all
        for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
        {
          SpritePoser *pSprite = Sprites[iSprite];

          Matrix44 mSpriteScale = Matrix44::mIdentity;
          mSpriteScale.Scale(pSprite->m_vScale);
          Matrix44 mSpriteScaleInv = Inverse(mSpriteScale);

          // calculate positions in relative coordinates
          Vector3D vIntRel = Transform(mSpriteScaleInv, vIntersection);
          Vector3D vPointRel = Transform(mSpriteScaleInv, m_vMovingPoint);

          // transform delta to absolute coordinates
          Vector3D vDelta = Transform(mSpriteScale, (vIntRel - vPointRel));
          if(m_iMovingLock >= 0)
          {
            for(int i = 0; i < 3; i++)
              if(i != m_iMovingLock)
                vDelta[i] = 0;
          }

          pSprite->m_vPosition += vDelta;
        }
        m_vMovingPoint = vIntersection;

        if(GetAnimator()->GetAutoSetFrame())
          GetAnimator()->GetAnimWindow()->OnAddFrame();
        GetAnimator()->FileChanged();
        return;
      }

    }
    else
    {
      m_bMoving = false;
    }
  }

};
