#pragma once

/// Tool for rotating sprites
class RotateTool : public AnimatorTool
{
public:
  bool m_bRotating;
  int m_iRotatingLock;
  Vector3D m_vMovingPoint, m_vOriginalPoint;
  Vector3D m_vMovingNormal;

public:
  RotateTool()
  {
    m_bRotating = false;
    m_iRotatingLock = -1;
  }

  void Run(float fDeltaTime)
  {
    const std::vector<SpritePoser *> &Sprites = GetAnimator()->GetSelected();
    if(Sprites.empty())
    {
      m_bRotating = false;
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

    bool bShortcutDown = GetAnimator()->IsKeyDown(VK_CONTROL) && GetAnimator()->IsKeyDown(VK_SHIFT);

    float fCircleWidth = 2.0f;
    float fCircleRadius = 10.0f;
    Vector3D vAxisHitPos;
    int iSelectedAxis = -1;
    if(bShortcutDown && !m_bRotating)
    {
      float fNearestCircleDist = FLT_MAX;
      for(int i = 0; i < 3; i++)
      {
        int iHitAxis = 0;
        float fHitDist = 0.0f;

        // select with mouse over
        Vector3D vBoxHalfSize(fCircleRadius,fCircleRadius,fCircleRadius);
        vBoxHalfSize[i] = fCircleWidth;
        if(IntersectRayBox(vCursorPos, vCursorDir, vCenterPos, vBoxHalfSize, fHitDist, iHitAxis))
        {
          if(vCursorDir[i] == 0)
            continue;

          // get hit point on plane
          // CursorPos.x + vCursorDir.x * fT == vCenterPos.x
          float fT = (vCenterPos[i] - vCursorPos[i]) / vCursorDir[i];

          vAxisHitPos = vCursorPos + vCursorDir * fT;

          // test that is within circle radius
          float fDistFromCenter = (vAxisHitPos - vCenterPos).Length();

          float fDistFromCircle = fabsf(fDistFromCenter - fCircleRadius);
          if(fDistFromCircle > fCircleWidth)
            continue;

          if(fDistFromCircle < fNearestCircleDist)
          {
            fNearestCircleDist = fDistFromCircle;
            iSelectedAxis = i;
          }          
        }
      }

      for(int i = 0; i < 3; i++)
      {
        Vector3D vRotationAxis(0,0,0);
        vRotationAxis[i] = 1;
        int iCol[3] = {0,0,0};
        iCol[i] = 255;

        if(iSelectedAxis == i)
        {
          iCol[0] += 192;
          iCol[1] += 192;
          iCol[2] += 192;
          iCol[i] = 255;
        }

        // draw circle
        Vector3D vDirX(0,0,0);
        vDirX[(i+1)%3] = 1;
        Vector3D vDirY(0,0,0);
        vDirY[(i+2)%3] = 1;
        DrawCircle(vCenterPos, vDirX, vDirY, fCircleRadius, 16.0f, iCol[0], iCol[1], iCol[2], false);
        DrawArrow(vCenterPos + vDirY * fCircleRadius, vCenterPos + vDirY * fCircleRadius + vDirX, 2.0f, vRotationAxis, iCol[0], iCol[1], iCol[2], false);
        DrawArrow(vCenterPos - vDirY * fCircleRadius, vCenterPos - vDirY * fCircleRadius - vDirX, 2.0f, vRotationAxis, iCol[0], iCol[1], iCol[2], false);
      }

    }
    else if(bShortcutDown && m_bRotating && m_iRotatingLock >= 0)
    {
      // draw axis currently rotated around
      Vector3D vRotationAxis(0,0,0);
      vRotationAxis[m_iRotatingLock] = 1;
      int iCol[3] = {0,0,0};
      iCol[m_iRotatingLock] = 255;
      Vector3D vDirX(0,0,0);
      vDirX[(m_iRotatingLock+1)%3] = 1;
      Vector3D vDirY(0,0,0);
      vDirY[(m_iRotatingLock+2)%3] = 1;
      DrawCircle(vCenterPos, vDirX, vDirY, fCircleRadius, 16.0f, iCol[0], iCol[1], iCol[2], false);
      DrawArrow(vCenterPos + vDirY * fCircleRadius, vCenterPos + vDirY * fCircleRadius + vDirX, 2.0f, vRotationAxis, iCol[0], iCol[1], iCol[2], false);
      DrawArrow(vCenterPos - vDirY * fCircleRadius, vCenterPos - vDirY * fCircleRadius - vDirX, 2.0f, vRotationAxis, iCol[0], iCol[1], iCol[2], false);
      DrawLine(vCenterPos, vCenterPos + Normalize(m_vMovingPoint - vCenterPos) * fCircleRadius, iCol[0], iCol[1], iCol[2], false);
      DrawLine(vCenterPos, vCenterPos + Normalize(m_vOriginalPoint - vCenterPos) * fCircleRadius, iCol[0], iCol[1], iCol[2], false);
    }
    else if(bShortcutDown && m_bRotating)
    {
      DrawLine(vCenterPos, m_vMovingPoint, 255, 0, 255, false);
      DrawLine(vCenterPos, m_vOriginalPoint, 255, 0, 255, false);
    }

    // holding ctrl + shift + mouse
    if(bShortcutDown && GetAnimator()->IsMouseLeftDown())
    {
      // just started
      if(!m_bRotating)
      {
        // circle axis
        if(iSelectedAxis >= 0)
        {
          m_vMovingNormal = Vector3D(0,0,0);
          if(iSelectedAxis == 0)
            m_vMovingNormal.x = -Sign(vCursorDir.x);
          else if(iSelectedAxis == 1)
            m_vMovingNormal.y = -Sign(vCursorDir.y);
          else if(iSelectedAxis == 2)
            m_vMovingNormal.z = -Sign(vCursorDir.z);
          m_bRotating = true;
          m_iRotatingLock = iSelectedAxis;

          // initialize point
          float fDot = Dot(m_vMovingNormal, vCursorDir);
          float fPlaneD = -Dot(vCenterPos, m_vMovingNormal);
          float fIntersectionDist = -(Dot(vCursorPos, m_vMovingNormal) + fPlaneD) / fDot;
          m_vMovingPoint = vCursorPos + vCursorDir * fIntersectionDist;
          m_vOriginalPoint = m_vMovingPoint;
        }

        // ray to voxels
        if(!m_bRotating)
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
                m_vOriginalPoint = m_vMovingPoint;
                m_vMovingNormal = -GetAnimator()->GetCameraMatrix().GetFrontVector();
                m_iRotatingLock = -1;
                m_bRotating = true;
              }
            }
          }
        }

        // ray to bounding box
        if(!m_bRotating)
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
                m_vOriginalPoint = m_vMovingPoint;
                m_vMovingNormal = -GetAnimator()->GetCameraMatrix().GetFrontVector();
                m_iRotatingLock = -1;
                m_bRotating = true;
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
        Vector3D vPlanePos = m_vMovingPoint;
        if(m_iRotatingLock >= 0)
          vPlanePos = vCenterPos;
        float fPlaneD = -Dot(vPlanePos, m_vMovingNormal);
        float fIntersectionDist = -(Dot(vCursorPos, m_vMovingNormal) + fPlaneD) / fDot;
        Vector3D vIntersection = vCursorPos + vCursorDir * fIntersectionDist;

        // rotate with mouse delta
        Vector3D vDelta1 = Normalize(m_vMovingPoint - vCenterPos);
        Vector3D vDelta2 = Normalize(vIntersection - vCenterPos);
        if(m_iRotatingLock >= 0)
        {
          vDelta1[m_iRotatingLock] = 0;
          vDelta1.Normalize();
          vDelta2[m_iRotatingLock] = 0;
          vDelta2.Normalize();
        }
        Vector3D vAxis = Normalize(Cross(vDelta2, vDelta1));
        float fAngle = acosf(Clamp(Dot(vDelta2, vDelta1), 0.0f, 1.0f));
        if(fAngle < 0.0001f)
          return;

        Quaternion qRot = QuaternionFromAxisAngle(vAxis, fAngle);
        Matrix44 mTrans = MatrixFromQuaternion(qRot);

        for(unsigned int iSprite = 0; iSprite < Sprites.size(); iSprite++)
        {
          SpritePoser *pSprite = Sprites[iSprite];

          // add rotation
          pSprite->m_qRotation *= qRot;
          pSprite->m_qRotation.Normalize();

          // new rotated position
          Vector3D vNewPos = Transform3x3(mTrans, pSprite->m_vPosition - vCenterPos) + vCenterPos;

          // calculate positions with scale
          Matrix44 mSpriteScale = Matrix44::mIdentity;
          mSpriteScale.Scale(pSprite->m_vScale);
          Matrix44 mSpriteScaleInv = Inverse(mSpriteScale);
          Vector3D vPosRel = Transform(mSpriteScaleInv, pSprite->m_vPosition);
          Vector3D vNewPosRel = Transform(mSpriteScaleInv, vNewPos);
          Vector3D vDelta = Transform(mSpriteScale, (vNewPosRel - vPosRel));

          // move position
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
      m_bRotating = false;
    }
  }

};
