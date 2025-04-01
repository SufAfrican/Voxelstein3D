#pragma once

inline Vector3D GetLineLineDist(const Vector3D &u, const Vector3D &v, const Vector3D &p1, const Vector3D &p2)
{
  Vector3D w = p1 - p2;
  float b = Dot(u,v);
  float d = Dot(u,w);
  float e = Dot(v,w);
  return w + ((b*e - d) * u - (e - b*d) * v) / (1 - b*b);
}

/// Tool for scaling sprites
class ScaleTool : public AnimatorTool
{
public:
  bool m_bMoving;
  int m_iMovingLock;

public:
  ScaleTool()
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
    bool bShortcutDown = GetAnimator()->IsKeyDown(VK_CONTROL) && GetAnimator()->IsKeyDown(VK_MENU) && !GetAnimator()->IsKeyDown(VK_SHIFT);
    int iSelectedAxis = -1;
    if(bShortcutDown && !m_bMoving)
    {
      float fNearestDist = FLT_MAX;

      // draw preview arrows
      for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
      {
        Matrix44 mRot = MatrixFromQuaternion(Sprites[iSprite]->m_qRotation);

        for(int i = 0; i < 3; i++)
        {
          int iHitAxis = 0;
          float fAxisHitDist = 0.0f;
          Vector3D vBoxHalfSize = 0.5f * Vector3D(fArrowWidth, fArrowWidth, fArrowWidth);
          vBoxHalfSize[i] = 0.5f * fArrowLength;
          
          Vector3D vBoxCenter = Transform3x3(mRot, vBoxHalfSize);
          vBoxCenter += Sprites[iSprite]->m_vPosition;

          // transform AABB
          Vector3D vExtentX(mRot.m[0][0] * vBoxHalfSize.x, mRot.m[0][1] * vBoxHalfSize.x, mRot.m[0][2] * vBoxHalfSize.x);
          vExtentX.x = fabsf(vExtentX.x);
          vExtentX.y = fabsf(vExtentX.y);
          vExtentX.z = fabsf(vExtentX.z);

          Vector3D vExtentY(mRot.m[1][0] * vBoxHalfSize.y, mRot.m[1][1] * vBoxHalfSize.y, mRot.m[1][2] * vBoxHalfSize.y);
          vExtentY.x = fabsf(vExtentY.x);
          vExtentY.y = fabsf(vExtentY.y);
          vExtentY.z = fabsf(vExtentY.z);

          Vector3D vExtentZ(mRot.m[2][0] * vBoxHalfSize.z, mRot.m[2][1] * vBoxHalfSize.z, mRot.m[2][2] * vBoxHalfSize.z);
          vExtentZ.x = fabsf(vExtentZ.x);
          vExtentZ.y = fabsf(vExtentZ.y);
          vExtentZ.z = fabsf(vExtentZ.z);

          vBoxHalfSize.x = vExtentX.x + vExtentY.x + vExtentZ.x;
          vBoxHalfSize.y = vExtentX.y + vExtentY.y + vExtentZ.y;
          vBoxHalfSize.z = vExtentX.z + vExtentY.z + vExtentZ.z;

          // select with mouse over
          if(IntersectRayBox(vCursorPos, vCursorDir, vBoxCenter, vBoxHalfSize, fAxisHitDist, iHitAxis))
          {
            Vector3D vLineDir(0,0,0); vLineDir[i] = 1;
            vLineDir = Transform3x3(mRot, vLineDir);

            Vector3D vDist = GetLineLineDist(vLineDir, vCursorDir, vCursorPos, Sprites[iSprite]->m_vPosition);
            float fPointDistanceFromLine = vDist.Length();

            // didn't actually hit
            if(fPointDistanceFromLine > fArrowWidth)
              continue;

            if(fPointDistanceFromLine < fNearestDist)
            {
              fNearestDist = fPointDistanceFromLine;
              iSelectedAxis = i;
            }
          }
        }
      }

      for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
      {
        Matrix44 mRot = MatrixFromQuaternion(Sprites[iSprite]->m_qRotation);

        for(int i = 0; i < 3; i++)
        {
          int iCol[3] = {0,0,0};
          iCol[i] = 255;
          Vector3D vLineDir(0,0,0); vLineDir[i] = 1;
          vLineDir = Transform3x3(mRot, vLineDir);

          if(iSelectedAxis == i)
          {
            iCol[0] += 192;
            iCol[1] += 192;
            iCol[2] += 192;
            iCol[i] = 255;
          }
           
          for(int i = 0; i < 10; i++)
            DrawArrow(Sprites[iSprite]->m_vPosition + vLineDir * fArrowLength * (i / 10.0f), Sprites[iSprite]->m_vPosition + vLineDir * fArrowLength * ((i + 0.5f) / 10.0f), 0.1f * fArrowWidth, GetAnimator()->GetCameraMatrix().GetFrontVector(), iCol[0], iCol[1], iCol[2], false);
        }
      }
    }
    else if(bShortcutDown && m_bMoving && m_iMovingLock >= 0)
    {
      // draw axis currently scaled along
      for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
      {
        Vector3D vBoxSize = Vector3D(fArrowWidth, fArrowWidth, fArrowWidth);
        vBoxSize[m_iMovingLock] = fArrowLength;
        int iCol[3] = {0,0,0};
        iCol[m_iMovingLock] = 255;
        Vector3D vLineDir(0,0,0); vLineDir[m_iMovingLock] = 1;
        vLineDir = Transform3x3(MatrixFromQuaternion(Sprites[iSprite]->m_qRotation), vLineDir);
        for(int i = 0; i < 10; i++)
          DrawArrow(Sprites[iSprite]->m_vPosition + vLineDir * fArrowLength * (i / 10.0f), Sprites[iSprite]->m_vPosition + vLineDir * fArrowLength * ((i + 0.5f) / 10.0f), 0.1f * fArrowWidth, GetAnimator()->GetCameraMatrix().GetFrontVector(), iCol[0],iCol[1],iCol[2], false);
      }
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
          m_iMovingLock = iSelectedAxis;
          m_bMoving = true;
        }
        else
        {
          m_iMovingLock = -1;
          m_bMoving = true;
        }
        return;
      }
      else
      {
        // move with mouse delta
        float fDelta;
        fDelta = GetAnimator()->GetMouse().x * 0.01f;

        // scale all
        for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
        {
          SpritePoser *pSprite = Sprites[iSprite];

          if(m_iMovingLock >= 0)
          {
            pSprite->m_vScale[m_iMovingLock] += fDelta;
          }
          else
          {
            float fOldScale = pSprite->m_vScale.x;
            pSprite->m_vScale.x += fDelta;
            float fFactor = pSprite->m_vScale.x / fOldScale;
            pSprite->m_vScale.y *= fFactor;
            pSprite->m_vScale.z *= fFactor;


          }
        }

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
