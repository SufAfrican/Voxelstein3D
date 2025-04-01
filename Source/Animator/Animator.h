#pragma once

class CameraTool;
class AnimatedSprite;
class SpritePoser;
class AnimationFile;
class AnimWindow;
class ToolWindow;
class AnimatorTool;

/// Main program class
class Animator
{
public:
  Animator();
  ~Animator();
  /// Creates stuff dependent on main window
  void CreateWindows(void);

  /// Creates a new file
  void NewFile(void);
  /// Opens an existing file
  void OpenFile(void);
  /// Saves file
  bool SaveFile(void);
  /// Saves file to new location
  bool SaveFileAs(void);

  /// Asks quit confirmation
  bool CanExit(void);

  /// Loads a new sprite
  void AddNewSprite(void);
  /// Replaces selected sprite
  void ReplaceSprite(void);
  /// Deletes selected sprites
  void DeleteSprite(void);
  /// Changes pivot of selected sprite
  void ChangeSpritePivot(void);
  /// Copies selected sprites poses
  void CopyPoses(void);
  /// Pastes pose to selected sprites
  void PastePoses(void);
  /// Applies pose of selected sprites to all frames
  void ApplyPosesToAll(void);
  /// Applies current pose of sprite[0] relative to sprite[1] to all frames
  void ApplyRelativePosesToAll(void);
  /// Opens dialog for editing position
  void EditPosition(void);
  /// Opens dialog for editing rotation
  void EditRotation(void);
  /// Opens dialog for editing scale
  void EditScale(void);

  /// Enables/disables physics editing mode
  void TogglePhysicsMode(void);
  /// Enables/disables physics preview mode
  void TogglePhysicsPreviewMode(void);
  /// Adds a particle to selected sprite
  void AddParticle(void);
  /// Removes selected particles
  void RemoveParticle(void);
  /// Toggles collision on selected particles
  void ToggleParticleCollision(bool bEnabled);
  /// Connects selected particles
  void ConnectParticles(bool bConnect);
  /// Sets connection activation distance
  void SetConnectDistance(void);
  /// Sets selected particles as movepoint particles
  void SetParticleAsMovePoint(bool bSet);
  /// Removes movepoints from sprite
  void RemoveMovePointsFromSprite(void);
  /// Sets selected particle parent
  void SetParticleParent(void);
  /// Copies all physics data
  void CopyPhysics(void);
  /// Pastes all physics data
  void PastePhysics(void);

  /// Resets camera position
  void ResetCamera(void);
  /// Sets camera to fps view
  void SetFPSView(void);
  /// Toggles grid visibility
  void ToggleGrid(void);


  /// Returns camera rotation and translation matrix
  inline Matrix44 &GetCameraMatrix(void) { return m_mCamera; }

  /// Sets mouse input delta
  inline void SetMouse(float fX, float fY, bool bDownLeft, bool bDownRight) { m_vMouse = Vector2D(fX, fY); m_bMouseLeft = bDownLeft; m_bMouseRight = bDownRight; }
  /// Returns mouse input delta
  inline const Vector2D &GetMouse(void) { return m_vMouse; }

  /// Runs the animator
  void Run(float fDeltaTime);

  /// Returns true if key was clicked
  bool IsKeyClicked(int iVirtualKey);
  /// Returns true if key is down
  bool IsKeyDown(int iVirtualKey, bool bInMainWindow = true, bool bInAnimWindow = false);

  /// Returns true if left mouse button is down
  bool IsMouseLeftDown(void);
  /// Returns true if right mouse button is down
  bool IsMouseRightDown(void);

  /// Enables/disables frame autoset
  inline void SetAutoSetFrame(bool bEnabled) { m_bAutoSetFrame = bEnabled; }
  /// Returns whether frame autoset is enabled
  inline bool GetAutoSetFrame(void) { return m_bAutoSetFrame; }

  /// Returns tools
  inline const std::vector<AnimatorTool *> &GetTools(void) { return m_Tools; }
  /// Returns animation window
  inline AnimWindow *GetAnimWindow(void) { return m_pAnimWindow; }
  /// Returns animation instance
  inline AnimatedSprite *GetAnimation(void) { return m_pInstance; }
  /// Returns animation file
  inline AnimationFile *GetFile(void) { return m_pFile; }
  /// Returns all sprites
  inline std::vector<SpritePoser *> GetSprites(void) { if(m_bPhysicsMode && m_iSpritePicking == 0) return m_ParticlePosers; else return m_pInstance->m_Sprites; }
  /// Returns selected sprites
  inline std::vector<SpritePoser *> &GetSelected(void) { if(m_bPhysicsMode && m_iSpritePicking == 0) return m_SelectedParticles; else return m_Selected; }
  /// Selects a new sprite, or NULL to clear selection
  void Select(SpritePoser *pSprite);
  /// Selects all sprites
  void SelectAll(void);
  /// Draws sprite picking visualization
  void DrawSpritePickingVisualization(SpritePoser *pSprite);
  /// Returns true if in physics editing mode
  inline bool IsInPhysicsMode(void) { return m_bPhysicsMode; }
  /// Returns true if is picking sprites in physics editing mode
  inline bool IsInSpritePickingMode(void) { return m_bPhysicsMode && m_iSpritePicking > 0; }

  /// Returns cursor position in world coordinates
  Vector3D GetCursorPosition(void);

  /// Marks the file as modified
  inline void FileChanged(void) { m_bFileChanged = true; }

private:
  /// Unloads the file
  void UnloadFile(void);
  /// Draws grid
  void DrawGrid(void);
  /// Returns index of sprite in file
  bool GetSpriteIndex(SpritePoser *pSprite, unsigned int &iIndex);
  /// Creates particle posers
  void CreateParticlePosers(void);
  /// Destroys particle posers
  void DestroyParticlePosers(void);
  /// Returns index to link array from poser
  unsigned int GetParticleLinkIndex(SpritePoser *pPoser);
  /// Returns poser for given particle
  SpritePoser *GetParticlePoser(unsigned int iSpritePhysics, unsigned int iParticle);
  /// Called after picking sprite
  void SetParticleAsMovePoint_internal(void);
  /// Called after picking sprite
  void AddParticle_internal(void);
  /// Called after picking sprite
  void SetParticleParent_internal(void);
  /// Called after picking sprite
  void RemoveMovePointsFromSprite_internal(void);

private:

  /// Mouse input delta
  Vector2D m_vMouse;
  /// Mouse button down
  bool m_bMouseLeft;
  /// Mouse button down
  bool m_bMouseRight;
  /// Camera rotation and translation
  Matrix44 m_mCamera;

  /// Sprite instance
  AnimatedSprite *m_pInstance;
  /// Sprite file
  AnimationFile *m_pFile;

  /// Selected sprites
  std::vector<SpritePoser *> m_Selected;

  /// Physics editing mode
  bool m_bPhysicsMode;
  /// Link from sprite poser to particle
  struct ParticlePoserLink
  {
    unsigned int iSpritePhysics;
    unsigned int iParticle;
  };
  /// Sprites that visualize particles
  std::vector<SpritePoser *> m_ParticlePosers;
  /// Links from posers to actual particles
  std::vector<ParticlePoserLink> m_ParticlePoserLinks;
  /// Selected particle visualizations
  std::vector<SpritePoser *> m_SelectedParticles;
  /// Sprite picking in physics editing mode
  int m_iSpritePicking;

  /// Sprite picking modes
  enum SpritePicking
  {
    SpritePicking_SetParticleAsMovePoint = 1,
    SpritePicking_AddParticle = 2,
    SpritePicking_SetParticleParent = 3,
    SpritePicking_RemoveMovePointsFromSprite = 4
  };

  /// Animation editing window
  AnimWindow *m_pAnimWindow;

  /// Camera movement tool
  CameraTool *m_pCameraTool;
  /// Sprite editing tools
  std::vector<AnimatorTool *> m_Tools;
  /// Automatically add frames
  bool m_bAutoSetFrame;

  /// Grid is rendered
  bool m_bRenderGrid;

  /// Last frame time
  float m_fTime;

  /// Currently opened file
  std::string m_strFilename;
  /// Has the file been changed
  bool m_bFileChanged;

  /// Delay between ticks (seconds)
  float m_fTickDuration;
  /// Time when last tick was executed
  float m_fLastTick;

  /// Sprite files to save (pivot changed)
  std::vector<std::string> m_SaveSprites;
};

extern Animator *g_pAnimator;
inline Animator *GetAnimator(void) { return g_pAnimator; }