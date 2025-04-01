#pragma once

/// Verlet integrated particle physics solver ala Thomas Jakobsen
class PhysicsSolver
{
public:
  /// How many ticks a particle should stay still before falling asleep
  static const unsigned int iSleepTickCount = 50;
public:
  PhysicsSolver();
  ~PhysicsSolver();

  /// Physically simulated particle
  struct Particle
  {
    Particle(const Vector3D &vPos, float fRadius) : vPos(vPos), vOldPos(vPos),
      fRadius(fRadius), vConstraintMove(0,0,0),
      iSleepCounter(0), vFriction(0,0,0) {}

    /// Current position
    Vector3D vPos;
    /// Last position
    Vector3D vOldPos;
    /// Temporary variable that holds position offset caused by constraints
    Vector3D vConstraintMove;

    /// Collision radius of particle, less than 0.0f for no collision
    float fRadius;
    /// Amount of ticks particle has been asleep for
    char iSleepCounter;

    /// Current friction
    Vector3D vFriction;

    /// Returns true if particle is sleeping
    __forceinline bool IsSleeping(void) { return iSleepCounter >= PhysicsSolver::iSleepTickCount; }
    /// Wakes up the particle
    __forceinline void WakeUp(void) { iSleepCounter = 0; }
  };

  /// Physically simulated stick constraint
  struct Constraint
  {
    Constraint(unsigned int iP1, unsigned int iP2, float fLength) : iP1(iP1), iP2(iP2), fLength(fLength) {}
    /// Particle indices
    unsigned int iP1, iP2;
    /// Desired length between particles, negative for "minimum length"
    float fLength;
  };

  /// Box constructed from a set of particles and constraints
  struct Box
  {
    unsigned int iParticle[9];
    unsigned int iConstraint[36];
  };

  /// Adds a box to simulation
  void AddBox(Box &box, const Vector3D &vPosition, const Vector3D &vHalfSize);
  /// Returns box transformation matrix
  void GetTransform(Box &box, Matrix44 &m);
  /// Sets velocity of a box
  void SetVelocity(Box &box, const Vector3D &vVelocity);
  /// Removes box from simulation
  void RemoveBox(Box &box);

  /// Stick constructed from a set of particles and constraints
  struct Stick
  {
    unsigned int iParticle[3];
    unsigned int iConstraint[3];
    Vector3D vFront;
    Quaternion qRot;
  };

  /// Adds a stick to simulation
  void AddStick(Stick &stick, const Vector3D &vPosition, const Vector3D &vDir, float Length, float Radius);
  /// Returns stick transformation matrix
  void GetTransform(Stick &stick, Matrix44 &m);
  /// Sets velocity of a stick
  void SetVelocity(Stick &stick, const Vector3D &vVelocity);
  /// Removes stick from simulation
  void RemoveStick(Stick &stick);

  /// Adds a new particle to the simulation
  unsigned int AddParticle(const Vector3D &vPosition, float fRadius);
  /// Removes a particle from simulation
  void RemoveParticle(unsigned int iParticle);

  /// Adds a constraint to simulation
  unsigned int AddConstraint(unsigned int iParticle1, unsigned int iParticle2, float fLength = -FLT_MAX);
  /// Removes constraint from simulation
  void RemoveConstraint(unsigned int iConstraint);

  /// Returns pointer to particle
  inline Particle *GetParticle(unsigned int iParticle)
  {
    if(iParticle >= m_Particles.size())
      return NULL;
    return &m_Particles[iParticle];
  }

  /// Updates physics
  void Tick(void);

private:
  /// Clips particle movement
  inline Vector3D GetClippedMove(Particle &particle, const Vector3D &vMove);

public:
  /// Active particles (index to m_Particles)
  std::vector<unsigned int> m_ActiveParticles;
  /// Free particles (index to m_Particles)
  std::vector<unsigned int> m_FreeParticles;
  /// Active constraints (index to m_Constraints)
  std::vector<unsigned int> m_ActiveConstraints;
  /// Free constraints (index to m_Constraints)
  std::vector<unsigned int> m_FreeConstraints;
  /// Particle data
  std::vector<Particle> m_Particles;
  /// Constraint data
  std::vector<Constraint> m_Constraints;
};

extern PhysicsSolver *g_pPhysicsSolver;
inline PhysicsSolver *GetPhysics(void) { return g_pPhysicsSolver; }