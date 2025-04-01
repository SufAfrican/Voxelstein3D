#pragma once

struct VoxlapImage;
class GameMenuScreen;

/// Ingame menu
class GameMenu
{
public:
  GameMenu();
  ~GameMenu();

  /// Updates menu
  void Run(float fDeltaTime);

  /// Opens finishing screen
  void OpenFinishScreen(void);

  /// Returns true if menu is open
  inline bool IsOpen(void) { return !m_ScreenStack.empty(); }

public:
  /// Draws start screen
  void RunStartScreen(float fDeltaTime);
  /// Draws a tick box
  void DrawTickBox(bool bTicked, int iX, int iY, int iSize = 100);
  /// Returns true if any key is pressed down
  bool PressedAnyKey(void);

public:
  /// Box images
  VoxlapImage *m_pBoxTicked, *m_pBoxUnticked;
  /// Brightness for fading
  unsigned char m_iBrightness;

  /// Currently open menu screens
  std::vector<GameMenuScreen *> m_ScreenStack;
};

extern GameMenu *g_pGameMenu;
inline GameMenu *GetGameMenu(void) { return g_pGameMenu; }