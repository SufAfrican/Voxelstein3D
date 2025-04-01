#pragma once

/// Draws a line
void DrawLine(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bDepthTest = true);
/// Draws an arrow
void DrawArrow(const Vector3D &vStart, const Vector3D &vEnd, float fSize, const Vector3D &vFaceDir, int iR, int iG, int iB, bool bDepthTest = true);
/// Draws a circle
void DrawCircle(const Vector3D &vCenter, const Vector3D &vDirX, const Vector3D &vDirY, float fRadius, float fSlices, int iR, int iG, int iB, bool bDepthTest = true);
/// Draws lines of a box
void DrawBoxLines(const Vector3D &vStart, const Vector3D &vEnd, int iR, int iG, int iB, bool bDepthTest = true);
/// Returns world position of pixel coordinates
Vector3D GetWorldFromScreen(int iX, int iY, const Matrix44 &mCamera);