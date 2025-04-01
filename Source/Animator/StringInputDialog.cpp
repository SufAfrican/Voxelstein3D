#include "StdAfx.h"
#include "Animator/StringInputDialog.h"

extern HWND ghwnd;

bool StringInputDialog::Run(const std::string &strTitle)
{
  HWND hParent = ghwnd;

  // register window class
  //
  static bool bClassRegistered = false;
  if(!bClassRegistered)
  {
    WNDCLASSEX wc;
    ZeroMemory(&wc,sizeof(WNDCLASSEX));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "StringInputDialog";
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if(RegisterClassEx(&wc)==NULL)
      return false;

    bClassRegistered = true;
  }

  // create window
  //
  m_hWindow = CreateWindowEx(
	  WS_EX_TOOLWINDOW,
	  "StringInputDialog",
	  strTitle.c_str(),
	  WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
	  CW_DEFAULT, CW_DEFAULT, 100, 100,
	  NULL, NULL, NULL, NULL);

  if(m_hWindow == NULL)
    return false;

  SetWindowLongPtr(m_hWindow, GWLP_USERDATA, (LONG_PTR)this);



  m_hOK = CreateWindow("BUTTON", "OK", 
                  WS_CHILD | WS_VISIBLE, 
                  4,
                  4, 
                  60, 
                  28, 
                  m_hWindow, NULL, NULL, NULL);

  m_hCancel = CreateWindow("BUTTON", "Cancel", 
                  WS_CHILD | WS_VISIBLE, 
                  68,
                  4, 
                  60, 
                  28, 
                  m_hWindow, (HMENU)IDCANCEL, NULL, NULL);

  m_hInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", m_strText.c_str(), 
                  WS_CHILD | WS_VISIBLE, 
                  4,
                  4, 
                  200, 
                  22, 
                  m_hWindow, (HMENU)IDCANCEL, NULL, NULL);

  extern HFONT g_hFont;
  SendMessage(m_hOK, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hCancel, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hInput, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));

  // initialize position
  /*RECT rect;
  GetWindowRect(ghwnd, &rect);
  int iCenterX = (rect.left + rect.right) / 2;
  int iCenterY = (rect.top + rect.bottom) / 2;*/
  POINT pnt;
  GetCursorPos(&pnt);
  int iCenterX = pnt.x;
  int iCenterY = pnt.y;
  SetWindowPos(m_hWindow, 0, iCenterX - 200/2, iCenterY - 100/2, 200, 100, SWP_NOZORDER|SWP_SHOWWINDOW);

  EnableWindow(hParent, FALSE);

  SetFocus(m_hInput);

  // run message loop
  //
  MSG msg;
  m_iReturnCode = IDCANCEL;
  m_bRunning = true;
  while(m_bRunning)
  {
    if(PeekMessage(&msg,0,0,0,PM_REMOVE))
    {
      if(msg.hwnd != m_hWindow)
      {
        SetActiveWindow(m_hWindow);
      }

      if(!IsDialogMessage(m_hWindow, &msg))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  EnableWindow(hParent,TRUE);

  char strTemp[256] = {0};
  GetWindowText(m_hInput, strTemp, 256);
  m_strText = strTemp;
  DestroyWindow(m_hOK);
  DestroyWindow(m_hCancel);
  DestroyWindow(m_hInput);
  DestroyWindow(m_hWindow);

  return m_iReturnCode == IDOK;
}

LRESULT CALLBACK StringInputDialog::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  StringInputDialog *pDiag = NULL;
  pDiag = (StringInputDialog *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);
  if(pDiag == NULL) return DefWindowProc(hwnd,msg,wParam,lParam);

  if(msg == WM_CLOSE)
  {
    SetActiveWindow(ghwnd);
    pDiag->m_bRunning = false;
  }
  else if(msg==WM_COMMAND)
  {
    if(lParam==(LPARAM)pDiag->m_hOK || lParam==0)
    {
      pDiag->m_iReturnCode=IDOK;
      pDiag->m_bRunning=false;
    }
    else if(lParam==(LPARAM)pDiag->m_hCancel)
    {
      pDiag->m_iReturnCode=IDCANCEL;
      pDiag->m_bRunning=false;
    }
  }
  else if(msg == WM_SIZE)
  {
    RECT rect;
    GetClientRect(hwnd, &rect);
    int iWidth = rect.right - rect.left;
    int iHeight = rect.bottom - rect.top;

    SetWindowPos(pDiag->m_hInput, 0, 4, 4, iWidth - 8, 22, SWP_NOZORDER);
    SetWindowPos(pDiag->m_hCancel, 0, iWidth - 70, iHeight - 32, 65, 28, SWP_NOZORDER);
    SetWindowPos(pDiag->m_hOK, 0, iWidth - 118, iHeight - 32, 45, 28, SWP_NOZORDER);

    InvalidateRect(hwnd,NULL,TRUE);
    UpdateWindow(hwnd);
  }

  return DefWindowProc(hwnd,msg,wParam,lParam);
}