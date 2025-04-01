#pragma once

/// Tool for selecting sprites
class SelectionTool : public AnimatorTool
{
public:
  bool m_bSelecting;
  Vector3D m_vSelectHit;

public:
  SelectionTool()
  {
    m_bSelecting = false;
  }

  void Run(float fDeltaTime)
  {
    Vector3D vCursorPos = GetAnimator()->GetCursorPosition();
    Vector3D vCursorDir = Normalize(vCursorPos - GetAnimator()->GetCameraMatrix().GetTranslation());

    // change selection
    if(!GetAnimator()->IsKeyDown(VK_CONTROL) && (GetAnimator()->IsMouseLeftDown() || GetAnimator()->IsInSpritePickingMode()))
    {
      if(!m_bSelecting)
      {
        // select nearest sprite
        float fNearest = FLT_MAX;
        SpritePoser *pNearestSprite = NULL;

        // in physics mode test particle centers first
        if(GetAnimator()->IsInPhysicsMode() && !GetAnimator()->IsInSpritePickingMode())
        {
          for(unsigned int i = 0; i < GetAnimator()->GetSprites().size(); i++)
          {
            SpritePoser *pSprite = GetAnimator()->GetSprites()[i];
            float fHitDistance;
            int iHitAxis;
            if(IntersectRayBox(vCursorPos, vCursorDir, pSprite->GetSprite()->GetTransform().GetTranslation(), Vector3D(2.0f, 2.0f, 2.0f), fHitDistance, iHitAxis))
            {
              if(fHitDistance < fNearest)
              {
                fNearest = fHitDistance;
                pNearestSprite = pSprite;
              }
            }
          }
        }
        
        // select nearest sprite
        if(pNearestSprite == NULL)
        {
          for(unsigned int i = 0; i < GetAnimator()->GetSprites().size(); i++)
          {
            SpritePoser *pSprite = GetAnimator()->GetSprites()[i];
            Voxlap::HitInfo info;
            if(Voxlap::HitScan(pSprite->GetSprite(), vCursorPos, vCursorDir, 1000.0f, info))
            {
              if(info.fDistance < fNearest)
              {
                fNearest = info.fDistance;
                pNearestSprite = pSprite;
              }
            }
          }
        }

        if(GetAnimator()->IsInSpritePickingMode() && !GetAnimator()->IsMouseLeftDown())
        {
          // visualize future selection
          if(pNearestSprite)
          {
            GetAnimator()->DrawSpritePickingVisualization(pNearestSprite);
          }
          return;
        }

        if(pNearestSprite != NULL)
          m_vSelectHit = vCursorPos + vCursorDir * fNearest;

        // shift for multiselect
        if(GetAnimator()->IsKeyDown(VK_SHIFT))
        {
          if(pNearestSprite != NULL)
            GetAnimator()->Select(pNearestSprite);
        }
        else
        {
          // clear old selection
          if(!GetAnimator()->GetSelected().empty())
            GetAnimator()->Select(NULL);
          // select new
          GetAnimator()->Select(pNearestSprite);
        }

        m_bSelecting = true;
      }
    }
    else
    {
      m_bSelecting = false;
    }
  }
};
