#pragma once

/// Simple dialog with an edit box
class StringInputDialog
{
public:
  /// Opens the window, returns true if pressed OK
  bool Run(const std::string &strTitle);

public:
  std::string m_strText;

private:
  HWND m_hWindow, m_hOK, m_hCancel, m_hInput;
  int m_iReturnCode;
  bool m_bRunning;
  static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
};