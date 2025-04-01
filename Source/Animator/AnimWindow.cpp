#include "StdAfx.h"
#include "Animator/Resources.h"
#include "Animator/StringInputDialog.h"

extern HWND ghwnd;
extern HFONT g_hFont;

AnimWindow::AnimWindow()
{
  // register window class
  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(WNDCLASSEX));
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = MsgProc;
  wc.hInstance = GetModuleHandle(NULL);
  wc.hIcon = NULL;
  wc.lpszClassName = "AnimWindowClass";
  wc.hIconSm = NULL;
  wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE+1);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  if (RegisterClassEx(&wc) == NULL)
  {
    printf("AnimWindow - failed to register class\n");
    return;
  }


  // create window
  m_hWindow = CreateWindowEx(WS_EX_TOOLWINDOW, "AnimWindowClass", "Animation window",
                            WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
                            0, 0, 100, 100,
                            ghwnd, NULL, NULL, NULL );
  if(m_hWindow == NULL)
  {
    printf("AnimWindow - failed to create window\n");
    return;
  }
  SetWindowLongPtr(m_hWindow,GWLP_USERDATA,(LONG_PTR)this);

  // anim buttons
  m_hAddAnim = CreateWindow("BUTTON", "New", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hDelAnim = CreateWindow("BUTTON", "Delete", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hRenameAnim = CreateWindow("BUTTON", "Rename", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);

  // animation combo box
  m_hAnimCombo = CreateWindowEx(
        WS_EX_CLIENTEDGE,
		    "COMBOBOX",
		    "",
		    WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
		    0, 0, 100, 200,
		    m_hWindow, NULL, NULL, NULL);
  if(m_hAnimCombo == NULL)
  {
    printf("AnimWindow - failed to create combo\n");
    return;
  }
  UpdateAnimationBox();
  SendMessage(m_hAnimCombo, CB_SETCURSEL, 0, 0);
  m_hAnimText = CreateWindow("STATIC", "Animation", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);

  // timeline
  m_hTimeLine = CreateWindowEx(WS_EX_CLIENTEDGE, "STATIC", "", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  SetWindowLongPtr(m_hTimeLine,GWLP_USERDATA,(LONG_PTR)this);
  m_OriginalTimeLineProc = (WNDPROC)(LONG_PTR)GetWindowLongPtr(m_hTimeLine,GWLP_WNDPROC);
  SetWindowLongPtr(m_hTimeLine,GWLP_WNDPROC,(LONG_PTR)TimeLineMsgProc);
  m_hTimeLineText = CreateWindow("STATIC", "Timeline", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);

  // frame buttons
  m_hAddFrame = CreateWindow("BUTTON", "Set", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hDelFrame = CreateWindow("BUTTON", "Remove", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hAutoSetFrame = CreateWindow("BUTTON", "Autoset", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  SendMessage(m_hAutoSetFrame, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

  // duration
  m_hDuration = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "1", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hDurationText = CreateWindow("STATIC", "Duration", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);

  // play
  m_hPlayAnim = CreateWindow("BUTTON", "Play", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hStopAnim = CreateWindow("BUTTON", "Stop", WS_VISIBLE | WS_CHILD, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hLoopAnim = CreateWindow("BUTTON", "Looping", WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  m_hPlaySeparator = CreateWindowEx(0, "STATIC", "", WS_VISIBLE | WS_CHILD | SS_ETCHEDVERT, 0, 0, 100, 20, m_hWindow, NULL, NULL, NULL);
  SendMessage(m_hLoopAnim, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

  // change fonts
  SendMessage(m_hStopAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hPlayAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hLoopAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hDurationText, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hDuration, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hAnimText, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hTimeLineText, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hAnimCombo, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hAddFrame, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hDelFrame, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hAutoSetFrame, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hAddAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hDelAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));
  SendMessage(m_hRenameAnim, WM_SETFONT, (WPARAM)g_hFont, (LPARAM)MAKEWORD(0,TRUE));

  // finally, show the window with correct size
  RECT rect;
  GetWindowRect(ghwnd, &rect);
  int iWidth = 400;
  int iHeight = 230;
  SetWindowPos(m_hWindow, 0, rect.left - 100, rect.bottom + 10, iWidth, iHeight, SWP_NOZORDER);
  ShowWindow(m_hWindow, SW_SHOW);
}

AnimWindow::~AnimWindow()
{
  DestroyWindow(m_hWindow);
}

void AnimWindow::ToggleVisibility(void)
{
  if(IsWindowVisible(m_hWindow))
  {
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_VIEW_ANIMWINDOW,MF_UNCHECKED);
    ShowWindow(m_hWindow,SW_HIDE);
  }
  else
  {
    CheckMenuItem(GetMenu(ghwnd),ID_MAINMENU_VIEW_ANIMWINDOW,MF_CHECKED);
    ShowWindow(m_hWindow,SW_SHOW);
  }
}

void AnimWindow::UpdateAnimationBox(void)
{
  SendMessage(m_hAnimCombo, CB_RESETCONTENT, 0, 0);
  for(unsigned int i = 0; i < GetAnimator()->GetFile()->m_Animations.size(); i++)
  {
    const std::string &strName = GetAnimator()->GetFile()->m_Animations[i].strName;
    SendMessage(m_hAnimCombo, CB_ADDSTRING, 0, (LPARAM)strName.c_str()); 
  }
  SendMessage(m_hAnimCombo, CB_SETCURSEL, GetAnimator()->GetAnimation()->m_iCurrentAnim, 0);
}

void AnimWindow::OnResize(void)
{
  RECT rect;
  GetClientRect(m_hWindow, &rect);
  int iWidth = rect.right - rect.left;
  int iHeight = rect.bottom - rect.top;

  // anim box
  int iAnimBoxWidth = Min(iWidth - 8, 200);
  int iAnimBoxOffsetY = 22;
  SetWindowPos(m_hAnimCombo, 0, 4, iAnimBoxOffsetY, iAnimBoxWidth, 200, SWP_NOZORDER);
  SetWindowPos(m_hAnimText, 0, 4, 4, iAnimBoxWidth, 16, SWP_NOZORDER);

  // anim buttons
  SetWindowPos(m_hAddAnim, 0, iAnimBoxWidth + 8, iAnimBoxOffsetY, 40, 22, SWP_NOZORDER);
  SetWindowPos(m_hDelAnim, 0, iAnimBoxWidth + 8 + 44, iAnimBoxOffsetY, 50, 22, SWP_NOZORDER);
  SetWindowPos(m_hRenameAnim, 0, iAnimBoxWidth + 8 + 98, iAnimBoxOffsetY, 50, 22, SWP_NOZORDER);

  // timeline
  int iTimelineOffsetY = 60;
  SetWindowPos(m_hTimeLineText, 0, 4, iTimelineOffsetY, iWidth - 8, 16, SWP_NOZORDER);
  SetWindowPos(m_hTimeLine, 0, 4, iTimelineOffsetY + 15, 256 + 4 + 1, 22, SWP_NOZORDER);

  // frame buttons
  SetWindowPos(m_hAddFrame, 0,               4, iTimelineOffsetY + 42,  40, 22, SWP_NOZORDER);
  SetWindowPos(m_hDelFrame, 0,          4 + 44, iTimelineOffsetY + 42,  60, 22, SWP_NOZORDER);
  SetWindowPos(m_hAutoSetFrame, 0, 4 + 44 + 65, iTimelineOffsetY + 42, 100, 22, SWP_NOZORDER);

  // duration
  SetWindowPos(m_hDuration, 0, 274, iTimelineOffsetY + 15, 80, 22, SWP_NOZORDER);
  SetWindowPos(m_hDurationText, 0, 274, iTimelineOffsetY, 80, 16, SWP_NOZORDER);

  // play buttons
  int iPlayControlsOffsetX = (iWidth - 204);
  int iPlayControlsOffsetY = iHeight - 34;
  SetWindowPos(m_hPlaySeparator, 0, iPlayControlsOffsetX, iPlayControlsOffsetY, 200, 30, SWP_NOZORDER);
  SetWindowPos(m_hPlayAnim, 0, iPlayControlsOffsetX + 4, iPlayControlsOffsetY + 4, 50, 22, SWP_NOZORDER);
  SetWindowPos(m_hStopAnim, 0, iPlayControlsOffsetX + 60, iPlayControlsOffsetY + 4, 50, 22, SWP_NOZORDER);
  SetWindowPos(m_hLoopAnim, 0, iPlayControlsOffsetX + 120, iPlayControlsOffsetY + 4, 70, 22, SWP_NOZORDER);
}

void AnimWindow::OnTimeLineClick(int iMouseX, int iMouseY)
{
  int iWidth = GetTimeLineWidth();

  // set selector
  GetAnimator()->GetAnimation()->m_fCurrentTime = Clamp(iMouseX / (float)iWidth, 0.0f, 1.0f);
  // update animation
  GetAnimator()->GetAnimation()->Run(0.0f, true);

  // repaint
  InvalidateRect(m_hTimeLine, NULL, TRUE);
  UpdateWindow(m_hTimeLine);
}

void AnimWindow::OnAddAnim(void)
{
  char strName[256];
  sprintf(strName, "Untitled %i", GetAnimator()->GetFile()->m_Animations.size());
  StringInputDialog sid;
  sid.m_strText = strName;
  if(sid.Run("Add animation"))
  {
    GetAnimator()->GetAnimation()->m_iCurrentAnim = GetAnimator()->GetFile()->AddAnimation(sid.m_strText, 1.0f);
    UpdateAnimationBox();
    OnAnimSelect();
    if(GetAnimator()->GetAutoSetFrame())
      GetAnimator()->GetAnimWindow()->OnAddFrame();
    GetAnimator()->FileChanged();
  }
}

void AnimWindow::OnDelAnim(void)
{
  int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);
  GetAnimator()->GetFile()->RemoveAnimation(iAnim);
  UpdateAnimationBox();
  SendMessage(m_hAnimCombo, CB_SETCURSEL, 0, 0);
  OnAnimSelect();
  GetAnimator()->FileChanged();
}

void AnimWindow::OnRenameAnim(void)
{
  unsigned int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);
  if(iAnim < GetAnimator()->GetFile()->m_Animations.size())
  {
    StringInputDialog sid;
    sid.m_strText = GetAnimator()->GetFile()->m_Animations[iAnim].strName;
    if(sid.Run("Rename animation"))
    {
      GetAnimator()->GetFile()->m_Animations[iAnim].strName = sid.m_strText;
      UpdateAnimationBox();
      GetAnimator()->FileChanged();
    }
  }
}

void AnimWindow::OnAddFrame(void)
{
  int iWidth = GetTimeLineWidth();
  int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);
  unsigned int iFrame = GetAnimator()->GetFile()->AddFrame(iAnim, GetAnimator()->GetAnimation()->m_fCurrentTime, 1.0f/iWidth);
  GetAnimator()->GetFile()->SetPoses(iAnim, iFrame, GetAnimator()->GetAnimation()->m_Sprites);
  GetAnimator()->FileChanged();
}

void AnimWindow::OnDelFrame(void)
{
  int iWidth = GetTimeLineWidth();
  int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);
  unsigned int iFrame = GetAnimator()->GetFile()->GetFrame(iAnim, GetAnimator()->GetAnimation()->m_fCurrentTime, 1.0f/iWidth);
  GetAnimator()->GetFile()->RemoveFrame(iAnim, iFrame);
  GetAnimator()->FileChanged();
}

void AnimWindow::OnAutoSetFrameChange(void)
{
  bool bChecked = SendMessage(m_hAutoSetFrame, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
  GetAnimator()->SetAutoSetFrame(bChecked);
}

void AnimWindow::OnAnimSelect(void)
{
  int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);
  GetAnimator()->GetAnimation()->m_iCurrentAnim = iAnim;
  // update duration
  char strTime[256] = {0};
  if((unsigned int)iAnim < GetAnimator()->GetFile()->m_Animations.size())
  {
    AnimationFile::Animation &anim = GetAnimator()->GetFile()->m_Animations[iAnim];
    sprintf(strTime, "%g", anim.fDuration); 
  }
  SetWindowText(m_hDuration, strTime);
  // update animation
  GetAnimator()->GetAnimation()->Run(0.0f, true);
  // repaint timeline
  InvalidateRect(m_hTimeLine, NULL, TRUE);
  UpdateWindow(m_hTimeLine);
  // refocus
  SetFocus(m_hWindow);
}

void AnimWindow::OnDurationChange(void)
{
  int iAnim = SendMessage(m_hAnimCombo, CB_GETCURSEL, 0, 0);

  // update duration
  char strTime[256] = {0};
  if((unsigned int)iAnim < GetAnimator()->GetFile()->m_Animations.size())
  {
    AnimationFile::Animation &anim = GetAnimator()->GetFile()->m_Animations[iAnim];
    GetWindowText(m_hDuration, strTime, 256);
    sscanf(strTime, "%f", &anim.fDuration);
    sprintf(strTime, "%g", anim.fDuration);
    SetWindowText(m_hDuration, strTime);
    GetAnimator()->FileChanged();
  }
}

void AnimWindow::OnPlayAnim(void)
{
  GetAnimator()->GetAnimation()->m_bPlaying = !GetAnimator()->GetAnimation()->m_bPlaying;
}

void AnimWindow::OnStopAnim(void)
{
  // rewind
  if(!GetAnimator()->GetAnimation()->m_bPlaying)
    OnTimeLineClick(0,0);
  GetAnimator()->GetAnimation()->m_bPlaying = false;
}

void AnimWindow::OnLoopAnimChange(void)
{
  bool bChecked = SendMessage(m_hLoopAnim, BM_GETCHECK, 0, 0) == BST_CHECKED ? true : false;
  GetAnimator()->GetAnimation()->m_bLooping = bChecked;
}

int AnimWindow::GetTimeLineWidth(void)
{
  /*RECT rect;
  GetClientRect(m_hTimeLine, &rect);
  int iWidth = rect.right - rect.left;*/
  return 256;
}

void AnimWindow::UpdateTimeLine(void)
{
  // repaint
  InvalidateRect(m_hTimeLine, NULL, TRUE);
  UpdateWindow(m_hTimeLine);
}

void AnimWindow::Update(void)
{
  UpdateAnimationBox();
  SendMessage(m_hAnimCombo, CB_SETCURSEL, 0, 0);
  OnAnimSelect();
}

LRESULT WINAPI AnimWindow::TimeLineMsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  AnimWindow *pWindow = NULL;
  pWindow = (AnimWindow *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);

  if(pWindow == NULL)
    return DefWindowProc( hwnd, msg, wParam, lParam );

  switch(msg)
  {
    case WM_PAINT:
    {
      CallWindowProc(pWindow->m_OriginalTimeLineProc, hwnd, msg, wParam, lParam);
      //HDC hdc = GetDC(hwnd);
      InvalidateRect(hwnd, NULL, FALSE);
      PAINTSTRUCT paint;
      HDC hdc = BeginPaint(hwnd, &paint);

      RECT rect;
      GetClientRect(hwnd, &rect);
      int iWidth = rect.right - rect.left;
      int iHeight = rect.bottom - rect.top;
      IntersectClipRect(hdc, 0,0,iWidth,iHeight);
      int iScaledWidth = pWindow->GetTimeLineWidth();

      // fill bg with white
      HBRUSH bg = CreateSolidBrush(RGB(255,255,255));
      FillRect(hdc, &rect, bg);

      // draw frames
      HPEN redPen = CreatePen(PS_SOLID, 1, RGB(255,0,0));
      HPEN oldPen = (HPEN)SelectObject(hdc, redPen);
      int iAnim = SendMessage(pWindow->m_hAnimCombo, CB_GETCURSEL, 0, 0);
      if((unsigned int)iAnim < GetAnimator()->GetFile()->m_Animations.size())
      {
        AnimationFile::Animation &anim = GetAnimator()->GetFile()->m_Animations[iAnim];
        std::vector<AnimationFile::Animation::Frame> &frames = anim.Frames;
        for(unsigned int i = 0; i < frames.size(); i++)
        {
          POINT points_line[2] = { {frames[i].fTime * iScaledWidth,0}, {frames[i].fTime * iScaledWidth,iHeight} };
          Polygon(hdc, points_line, 2);
        }
      }
      SelectObject(hdc, oldPen);
      DeleteObject((HGDIOBJ)redPen);

      // draw selector
      HGDIOBJ oldObj = SelectObject(hdc, GetSysColorBrush(COLOR_3DDKSHADOW));
      int iPos = GetAnimator()->GetAnimation()->m_fCurrentTime * iScaledWidth;
      POINT points_tri[3] = { {iPos,4}, {iPos-4,0}, {iPos+4,0} };
      Polygon(hdc, points_tri, 3);
      POINT points_line[2] = { {iPos,4}, {iPos,iHeight} };
      Polygon(hdc, points_line, 2);


      SelectObject(hdc, oldObj);
      DeleteObject((HGDIOBJ)bg);

      EndPaint(hwnd, &paint);

      return 0L;
    }
  }

  return CallWindowProc(pWindow->m_OriginalTimeLineProc, hwnd, msg, wParam, lParam);
}

LRESULT WINAPI AnimWindow::MsgProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
  AnimWindow *pWindow = NULL;
  pWindow = (AnimWindow *)(LONG_PTR)GetWindowLongPtr(hwnd,GWLP_USERDATA);

  if(pWindow == NULL)
    return DefWindowProc( hwnd, msg, wParam, lParam );

  switch(msg)
  {
    case WM_NOTIFY:
    {
    }
    break;

    case WM_COMMAND:
    {
      if(lParam == (LPARAM)pWindow->m_hAddAnim)
        pWindow->OnAddAnim();
      else if(lParam == (LPARAM)pWindow->m_hDelAnim)
        pWindow->OnDelAnim();
      else if(lParam == (LPARAM)pWindow->m_hRenameAnim)
        pWindow->OnRenameAnim();
      else if(lParam == (LPARAM)pWindow->m_hAddFrame)
        pWindow->OnAddFrame();
      else if(lParam == (LPARAM)pWindow->m_hDelFrame)
        pWindow->OnDelFrame();
      else if(lParam == (LPARAM)pWindow->m_hAutoSetFrame)
        pWindow->OnAutoSetFrameChange();
      else if(lParam == (LPARAM)pWindow->m_hAnimCombo && (HIWORD(wParam) == CBN_SELENDOK || HIWORD(wParam) == CBN_SELENDCANCEL))
        pWindow->OnAnimSelect();
      else if(lParam == (LPARAM)pWindow->m_hDuration && HIWORD(wParam) == EN_KILLFOCUS)
        pWindow->OnDurationChange();
      else if(lParam == (LPARAM)pWindow->m_hPlayAnim)
        pWindow->OnPlayAnim();
      else if(lParam == (LPARAM)pWindow->m_hStopAnim)
        pWindow->OnStopAnim();
      else if(lParam == (LPARAM)pWindow->m_hLoopAnim)
        pWindow->OnLoopAnimChange();
    }
    break;

    case WM_CLOSE:
    {
      pWindow->ToggleVisibility();
      return 0;
    }

    case WM_SIZE:
    {
      pWindow->OnResize();
    }
    break;

    case WM_KEYDOWN:
    {
      if(wParam == VK_LEFT)
      {
        // move one pixel left
        int iCurrent = GetAnimator()->GetAnimation()->m_fCurrentTime * pWindow->GetTimeLineWidth();
        pWindow->OnTimeLineClick(iCurrent - 1, 0);
      }
      else if(wParam == VK_RIGHT)
      {
        // move one pixel right
        int iCurrent = GetAnimator()->GetAnimation()->m_fCurrentTime * pWindow->GetTimeLineWidth();
        pWindow->OnTimeLineClick(iCurrent + 1, 0);
      }
    }
    break;

    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    {
      if(msg == WM_MOUSEMOVE && !(wParam & MK_LBUTTON))
        break;

      // test if in timeline
      POINT pnt;
      RECT rect;
      GetWindowRect(pWindow->m_hTimeLine, &rect);
      GetCursorPos(&pnt);
      if(pnt.x >= rect.left && pnt.x < rect.right
        && pnt.y >= rect.top && pnt.y < rect.bottom)
      {
        pWindow->OnTimeLineClick(pnt.x - rect.left, pnt.y - rect.top);
      }

      SetFocus(pWindow->m_hWindow);
    }
    break;
  }

  return DefWindowProc( hwnd, msg, wParam, lParam );
}