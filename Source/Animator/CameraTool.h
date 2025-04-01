#pragma once

/// Tool for moving camera, always active
class CameraTool : public AnimatorTool
{
public:
  /// Camera rotation in angles
  Vector3D m_vCameraRot;
  /// Camera position
  Vector3D m_vCameraPos;

public:
  CameraTool()
  {
    Reset();
  }

  void Reset(void)
  {
    m_vCameraRot = Vector3D(-2.35f,0.5f,0);
    m_vCameraPos = Vector3D(100,100,100);
  }

  void SetFPSView(void)
  {
    m_vCameraRot = Vector3D(0.0f,0.0f,0);
    m_vCameraPos = Vector3D(0,0,0);
  }

  void Run(float fDeltaTime)
  {
    Matrix44 &mCamera = GetAnimator()->GetCameraMatrix();

    if(GetAnimator()->IsMouseRightDown())
    {
      m_vCameraRot.x += GetAnimator()->GetMouse().x * 0.02f;
      m_vCameraRot.y += GetAnimator()->GetMouse().y * 0.02f;
      m_vCameraRot.x = WrapRadians(m_vCameraRot.x);
      m_vCameraRot.y = Clamp(m_vCameraRot.y, -Math::fPi * 0.499f, Math::fPi * 0.499f);
    }

    if(GetAnimator()->IsKeyDown('W') || GetAnimator()->IsKeyDown(VK_UP))
      m_vCameraPos += 100.0f * mCamera.GetFrontVector() * fDeltaTime;
    if(GetAnimator()->IsKeyDown('S') || GetAnimator()->IsKeyDown(VK_DOWN))
      m_vCameraPos -= 100.0f * mCamera.GetFrontVector() * fDeltaTime;
    if(GetAnimator()->IsKeyDown('A') || GetAnimator()->IsKeyDown(VK_LEFT))
      m_vCameraPos -= 100.0f * mCamera.GetSideVector() * fDeltaTime;
    if(GetAnimator()->IsKeyDown('D') || GetAnimator()->IsKeyDown(VK_RIGHT))
      m_vCameraPos += 100.0f * mCamera.GetSideVector() * fDeltaTime;

    mCamera.SetEuler(m_vCameraRot);
    mCamera.SetTranslation(m_vCameraPos);
  
}
};