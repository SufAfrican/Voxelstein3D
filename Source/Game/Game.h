#pragma once

class Entity;
class EntityTree;
class PlayerEntity;

/// Game!
class Game
{
public:
  Game();
  ~Game();

  /// Creates local player entity
  void CreatePlayer(const Vector3D &vPos);

  /// Adds a new entity to the game
  void AddEntity(Entity *pEntity);
  /// Removes an entity from the game (will be automatically deleted)
  void RemoveEntity(Entity *pEntity);

  /// Returns all entities of given class
  void GetEntities(int iClass, std::vector<Entity *> &out);
  /// Returns all entities inside given bounding box
  void GetEntities(const Vector3D &vStart, const Vector3D &vEnd, std::vector<Entity *> &out);
  /// Returns all entities
  inline const std::vector<Entity *> &GetEntities(void) { return m_Entities; }
  /// Returns local player entity
  inline PlayerEntity *GetLocalPlayer(void) { return m_pLocalPlayer; }

  /// Does a clip move against entities, returns position where no collision occurs
  Vector3D ClipMove(const Vector3D &vPosition, const Vector3D &vCollisionBox, const Vector3D &vMotion, unsigned int iCollisionTypeMask = -1);

  /// Updates game
  void Run(void);

  /// Returns camera position
  inline Vector3D GetCameraPos(void) { return m_mCamera.GetTranslation(); }
  /// Returns camera rotation and translation matrix
  inline Matrix44 &GetCameraMatrix(void) { return m_mCamera; }

  /// Sets mouse input delta
  inline void SetMouse(float fX, float fY) { m_vMouse = Vector2D(fX, fY); }
  /// Returns mouse input delta
  inline const Vector2D &GetMouse(void) { return m_vMouse; }

  /// Returns entity tree
  inline EntityTree *GetEntityTree(void) { return m_pEntityTree; }

  /// Key flags
  enum Key
  {
    Key_Forward = 0,
    Key_Backward = 1,
    Key_Left = 2,
    Key_Right = 3,
    Key_Jump = 4,
    Key_Fire = 5,
    Key_Use = 6,
    Key_Weapon1 = 7,
    Key_Weapon2 = 8,
    Key_Weapon3 = 9,
    Key_Weapon4 = 10,
    Key_Weapon5 = 11,
    Key_Weapon6 = 12,
    Key_Weapon7 = 13,
    Key_Weapon8 = 14,
    Key_Weapon9 = 15,
    Key_Crouch = 16,
    Key_Flashlight = 17,
    Key_NumKeys = 18,
  };

  /// Sets a key pressed/unpressed
  inline void SetKey(int iKey, bool bDown) { m_bKeyDown[iKey] = bDown; }
  /// Returns true if key is down
  inline bool IsKeyDown(int iKey) { return m_bKeyDown[iKey]; }

  /// Returns time (updated every frame)
  inline float GetTime(void) { return m_fTime; }
  /// Returns current tick
  inline unsigned int GetCurrentTick(void) { return m_iCurrentTick; }
  /// Returns delay between ticks in seconds
  inline float GetTickDuration(void) { return m_fTickDuration; }

  /// How many ticks in a second
  static const float fTicksPerSecond;
  /// How many ticks in a second
  static const unsigned int iTicksPerSecond;

  /// Entity was deleted, called by entity destructor
  void OnEntityDelete(Entity *pEntity);

  /// Returns true if flashlight is on
  inline bool IsFlashLightEnabled(void) { return m_bFlashLight; }
  /// Returns position of flashlight light
  inline Vector3D GetFlashLightPosition(void) { return m_vFlashLightPos; }
  /// Returns flashlight intensity
  inline float GetFlashLightIntensity(void) { return m_fFlashLightIntensity; }
  /// Sets flashlight on/off
  void SetFlashLight(bool bEnabled, const Vector3D &vPos, float fIntensity) { m_bFlashLight = bEnabled; m_vFlashLightPos = vPos; m_fFlashLightIntensity = fIntensity; }

  /// Gameplay stats
  struct Stats
  {
    Stats();
    /// Total number of voxels
    unsigned int iVoxels;
    /// Total number of enemies
    int iEnemies;
    /// Total number of pickup treasures
    int iTreasures;
    /// Total number of pickup items
    int iItems;
  };

  /// Returns gameplay stats for current level
  inline Stats &GetStats(void) { return m_Stats; }

  /// Finishes level
  inline void SetLevelFinished(void) { m_bLevelFinished = true; }
  /// Returns true if level is finished
  inline bool IsLevelFinished(void) { return m_bLevelFinished; }

private:
  /// Keys currently down
  bool m_bKeyDown[Key_NumKeys];

  /// Mouse input delta
  Vector2D m_vMouse;
  /// Camera rotation and translation
  Matrix44 m_mCamera;

  /// All entities in the game
  std::vector<Entity *> m_Entities;
  /// Entities to be removed once tick is over
  std::vector<Entity *> m_EntitiesPendingRemoval;
  /// Entities pending deletion once references have been removed
  std::vector<Entity *> m_RemovedEntities;

  /// Entity tree
  EntityTree *m_pEntityTree;

  /// Local player entity
  PlayerEntity *m_pLocalPlayer;

  /// Delay between ticks (seconds)
  float m_fTickDuration;
  /// Time when last tick was executed
  float m_fLastTick;
  /// Time updated every frame
  float m_fTime;
  /// Current tick
  unsigned int m_iCurrentTick;
  /// Is currently executing a tick
  bool m_bTicking;

  /// Gameplay stats for current level
  Stats m_Stats;
  /// Is level finished
  bool m_bLevelFinished;

  /// Is flashlight on
  bool m_bFlashLight;
  /// Flashlight position
  Vector3D m_vFlashLightPos;
  /// Flashlight intensity
  float m_fFlashLightIntensity;
};

extern Game *g_pGame;
inline Game *GetGame(void) { return g_pGame; }