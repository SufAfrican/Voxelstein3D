#pragma once

class ParticleSystem;

/// Handles recycling of particle systems
class ParticleSystemManager
{
public:
  ~ParticleSystemManager();

  /// @short Creates a new particle system (or returns a recycled one)
  /// @details Maximum count determines how many systems should be active at the same time. If the
  ///          count is exceeded the oldest system will be recycled even if it hasn't finished.
  template<class Type> inline Type *CreateSystem(unsigned int iMaxCount = -1);
  /// Returns number of running systems of given type
  template<class Type> inline unsigned int GetNumActiveSystems(void);
  /// Returns number of recyclable systems of given type
  template<class Type> inline unsigned int GetNumFinishedSystems(void);

  /// Updates manager
  void Update(void);

  /// Returns number of systems of given type
  void GetNumSystems(int iType, unsigned int &iNumTotal, unsigned int &iRecyclable);
  /// @short Returns an existing recyclable particle system
  /// @details When maximum count is used the function will return the oldest system even if it hasn't finished
  ParticleSystem *GetRecyclableSystem(int iType, unsigned int iMaxActiveCount = -1);
  /// Adds a new system so it can be later recycled
  void AddRecyclableSystem(ParticleSystem *pSystem, int iType);

public:
  typedef std::multimap<int, ParticleSystem *> ParticleMap;
  /// Map of recyclable systems
  ParticleMap m_ParticleMap;
};

extern ParticleSystemManager *g_pParticleSystemManager;
inline ParticleSystemManager *GetParticles(void) { return g_pParticleSystemManager; }

/////////////////////////////////////////////////////////////////////////////////////////

/// @short Creates a new particle system (or returns a recycled one)
/// @details Maximum count determines how many systems should be active at the same time. If the
///          count is exceeded the oldest system will be recycled even if it hasn't finished.
template<class Type> inline Type *ParticleSystemManager::CreateSystem(unsigned int iMaxCount)
{
  Type *pSystem = (Type *)GetRecyclableSystem(Type::m_Class, iMaxCount);
  if(pSystem == NULL)
  {
    pSystem = new Type();
    AddRecyclableSystem(pSystem, Type::m_Class);
  }
  pSystem->PrepareStart();
  return pSystem;
}

/// Returns number of running systems of given type
template<class Type> inline unsigned int ParticleSystemManager::GetNumActiveSystems(void)
{
  unsigned int iTotal, iFinished;
  GetNumSystems(Type::m_Class, iTotal, iFinished);
  return iTotal - iFinished;
}

/// Returns number of recyclable systems of given type
template<class Type> inline unsigned int ParticleSystemManager::GetNumFinishedSystems(void)
{
  unsigned int iTotal, iFinished;
  GetNumSystems(Type::m_Class, iTotal, iFinished);
  return iFinished;
}
