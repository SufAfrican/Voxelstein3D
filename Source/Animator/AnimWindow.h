#pragma once

/// Animation window with timeline etc.
class AnimWindow
{
public:
  AnimWindow();
  ~AnimWindow();

  /// Toggles window visibility
  void ToggleVisibility(void);

  /// Updates animation window after file change
  void Update(void);

  /// Updates animation box contents
  void UpdateAnimationBox(void);

  /// Window was resized
  void OnResize(void);
  /// Timeline was clicked
  void OnTimeLineClick(int iMouseX, int iMouseY);
  /// Redraws timeline
  void UpdateTimeLine(void);


  /// Add animation button was clicked
  void OnAddAnim(void);
  /// Delete animation button was clicked
  void OnDelAnim(void);
  /// Rename animation button was clicked
  void OnRenameAnim(void);
  /// Add frame button was clicked
  void OnAddFrame(void);
  /// Delete frame button was clicked
  void OnDelFrame(void);
  /// Auto set checkbox was clicked
  void OnAutoSetFrameChange(void);
  /// An animation was selected
  void OnAnimSelect(void);
  /// Duration was changed
  void OnDurationChange(void);
  /// Play animation button was clicked
  void OnPlayAnim(void);
  /// Stop animation button was clicked
  void OnStopAnim(void);
  /// Loop checkbox was clicked
  void OnLoopAnimChange(void);

  /// Returns clickable timeline width in pixels
  int GetTimeLineWidth(void);
public:
  /// Add frame button
  HWND m_hAddFrame;
  /// Delete frame button
  HWND m_hDelFrame;
  /// Autoset frame checkbox
  HWND m_hAutoSetFrame;
  /// Timeline
  HWND m_hTimeLine;
  /// Timeline text
  HWND m_hTimeLineText;
  /// Original WNDPROC for timeline
  WNDPROC m_OriginalTimeLineProc;

  /// Add anim button
  HWND m_hAddAnim;
  /// Animation text
  HWND m_hAnimText;
  /// Del anim button
  HWND m_hDelAnim;
  /// Rename anim button
  HWND m_hRenameAnim;
  /// Animation selection box
  HWND m_hAnimCombo;

  /// Separator line above play controls
  HWND m_hPlaySeparator;
  /// Play button
  HWND m_hPlayAnim;
  /// Stop button
  HWND m_hStopAnim;
  /// Looping check box
  HWND m_hLoopAnim;
  /// Duration input
  HWND m_hDuration;
  /// Duration text
  HWND m_hDurationText;

  /// Main window handle
  HWND m_hWindow;

  static LRESULT WINAPI MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
  static LRESULT WINAPI TimeLineMsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};