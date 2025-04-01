#pragma once

/// Voxel raytrace helper (straight port from Voxlap to maintain consistency)
class VoxelRayTraverse
{
public:
  /// Start position and direction
  inline VoxelRayTraverse(const Vector3D &vPos, const Vector3D &vDir) { Start(vPos, vDir); }

private:
  /// Fast float to integer function
  static _inline void ftol (float f, int *a)
  {
	  _asm
	  {
		  mov eax, a
		  fld f
		  fistp dword ptr [eax]
	  }
  }
public:

  /// Sets start position and direction
  inline void Start(const Vector3D &vPos, const Vector3D &vDir)
  {
    struct IntVec { int x; int y; int z; };
    IntVec h = { vPos.x, vPos.y, vPos.z };

    m_iDirSignX = Sign(vDir.x);
	  m_iDirSignY = Sign(vDir.y);
	  m_iDirSignZ = Sign(vDir.z);

	  int minz = Min(h.z,0);

    float kx, ky, kz;
	  float f = 0x3fffffff/1024; //Maximum delta value
	  if ((fabsf(vDir.x) >= fabs(vDir.y)) && (fabs(vDir.x) >= fabs(vDir.z)))
	  {
		  kx = 1024.0;
		  if (vDir.y == 0) ky = f; else ky = Min(fabsf(vDir.x/vDir.y)*1024.0f,f);
		  if (vDir.z == 0) kz = f; else kz = Min(fabsf(vDir.x/vDir.z)*1024.0f,f);
	  }
	  else if (fabs(vDir.y) >= fabs(vDir.z))
	  {
		  ky = 1024.0;
		  if (vDir.x == 0) kx = f; else kx = Min(fabsf(vDir.y/vDir.x)*1024.0f,f);
		  if (vDir.z == 0) kz = f; else kz = Min(fabsf(vDir.y/vDir.z)*1024.0f,f);
	  }
	  else
	  {
		  kz = 1024.0;
		  if (vDir.x == 0) kx = f; else kx = Min(fabsf(vDir.z/vDir.x)*1024.0f,f);
		  if (vDir.y == 0) ky = f; else ky = Min(fabsf(vDir.z/vDir.y)*1024.0f,f);
	  }

	  ftol(kx,&m_iDeltaX);
    ftol((vPos.x-(float)h.x)*kx,&m_iDirX);
    if (m_iDirSignX < 0)
      m_iDirX = m_iDeltaX+m_iDirX;

	  ftol(ky,&m_iDeltaY);
    ftol((vPos.y-(float)h.y)*ky,&m_iDirY);
    if (m_iDirSignY < 0)
      m_iDirY = m_iDeltaY+m_iDirY;

	  ftol(kz,&m_iDeltaZ);
    ftol((vPos.z-(float)h.z)*kz,&m_iDirZ);
    if (m_iDirSignZ >= 0)
      m_iDirZ = m_iDeltaZ-m_iDirZ;

    m_vPos = Vector3D(h.x, h.y, h.z);
    m_vDir = vDir;
  }

  /// Go to next cell, returns position
  inline const Vector3D &Step(void)
  {
		if((m_iDirZ <= m_iDirX) && (m_iDirZ <= m_iDirY))
		{
			m_vPos.z += m_iDirSignZ;
      m_iDirZ += m_iDeltaZ;
		}
		else
		{
			if(m_iDirX < m_iDirY)
			{
				m_vPos.x += m_iDirSignX;
        m_iDirX += m_iDeltaX;
			}
			else
			{
				m_vPos.y += m_iDirSignY;
        m_iDirY += m_iDeltaY;
			}
		}
    return m_vPos;
	}

  /// Returns current position
  inline const Vector3D &GetPos(void) { return m_vPos; }
  /// Returns ray direction
  inline const Vector3D &GetDir(void) { return m_vDir; }

private:
  /// Current position
  Vector3D m_vPos;
  /// Direction of ray
  Vector3D m_vDir;

  int m_iDeltaX, m_iDeltaY, m_iDeltaZ;
  int m_iDirX, m_iDirY, m_iDirZ;
  int m_iDirSignX, m_iDirSignY, m_iDirSignZ;
};