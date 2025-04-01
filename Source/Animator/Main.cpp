#include "StdAfx.h"
#include "Voxlap/msvc.h"
#include "Voxlap/sysmain.h"
#include "Voxlap/voxlap5.h"

#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

#include "Engine/VoxlapConversions.h"
#include "Animator/Resources.h"

HFONT g_hFont = NULL;
extern long (*catchwmclose)();
extern void (*MenuFunc)(int iID, HANDLE hComponent);

dpoint3d ipos, istr, ihei, ifor;

char bgcolor = 0;

// debug lines
struct DebugLine
{
  point3d vStart, vEnd;
  int iR,iG,iB;
  bool bPersistent;
};
extern std::vector<DebugLine> g_DebugLines;

// sprites
std::vector<vx5sprite *> g_Sprites;

void RegisterSprite(vx5sprite *sprite, bool bRenderInFront)
{
  std::vector<vx5sprite *> &Sprites = g_Sprites;
  Sprites.push_back(sprite);
}

void UnregisterSprite(vx5sprite *sprite, bool bRenderInFront)
{
  std::vector<vx5sprite *> &Sprites = g_Sprites;
  for(unsigned int i = 0; i < Sprites.size(); i++)
  {
    if(Sprites[i] == sprite)
    {
      // replace with last
      Sprites[i] = Sprites.back();
      Sprites.pop_back();
      break;
    }
  }
}

// animator
Animator *g_pAnimator = NULL;

long OnClose(void)
{
  if(!g_pAnimator->CanExit())
    return FALSE;
  return TRUE;
}

void OnMenuClick(int iID, HANDLE hComponent)
{
  switch(iID)
  {
    case ID_MAINMENU_FILE_NEW:
      {
        g_pAnimator->NewFile();
      }
      break;
    case ID_MAINMENU_FILE_OPEN:
      {
        g_pAnimator->OpenFile();
      }
      break;
    case ID_MAINMENU_FILE_SAVE:
      {
        g_pAnimator->SaveFile();
      }
      break;
    case ID_MAINMENU_FILE_SAVE_AS:
      {
        g_pAnimator->SaveFileAs();
      }
      break;
    case ID_MAINMENU_FILE_EXIT:
      {
        PostMessage(ghwnd, WM_CLOSE, 0, 0);
      }
      break;
    case ID_MAINMENU_VIEW_ANIMWINDOW:
      {
        g_pAnimator->GetAnimWindow()->ToggleVisibility();
      }
      break;
    case ID_MAINMENU_VIEW_RESETCAM:
      {
        g_pAnimator->ResetCamera();
      }
      break;
    case ID_MAINMENU_VIEW_SETFPSVIEW:
      {
        g_pAnimator->SetFPSView();
      }
      break;
    case ID_MAINMENU_LIMBS_ADD:
      {
        g_pAnimator->AddNewSprite();
      }
      break;
    case ID_MAINMENU_LIMBS_REPLACE:
      {
        g_pAnimator->ReplaceSprite();
      }
      break;
    case ID_MAINMENU_LIMBS_DEL:
      {
        g_pAnimator->DeleteSprite();
      }
      break;
    case ID_MAINMENU_LIMBS_PIVOT:
      {
        g_pAnimator->ChangeSpritePivot();
      }
      break;
    case ID_MAINMENU_EDIT_COPYPOSES:
      {
        g_pAnimator->CopyPoses();
      }
      break;
    case ID_MAINMENU_EDIT_PASTEPOSES:
      {
        g_pAnimator->PastePoses();
      }
      break;
    case ID_MAINMENU_EDIT_APPLYPOSETOALL:
      {
        g_pAnimator->ApplyPosesToAll();
      }
      break;
    case ID_MAINMENU_EDIT_POSITION:
      {
        g_pAnimator->EditPosition();
      }
      break;
    case ID_MAINMENU_EDIT_ROTATION:
      {
        g_pAnimator->EditRotation();
      }
      break;
    case ID_MAINMENU_EDIT_SCALE:
      {
        g_pAnimator->EditScale();
      }
      break;
    case ID_MAINMENU_EDIT_PARENT:
      {
        g_pAnimator->ApplyRelativePosesToAll();
      }
      break;
    case ID_MAINMENU_EDIT_SELECTALL:
      {
        g_pAnimator->SelectAll();
      }
      break;
    case ID_MAINMENU_EDIT_SELECTNONE:
      {
        g_pAnimator->Select(NULL);
      }
      break;
    case ID_MAINMENU_RAGDOLL_COPY:
      {
        g_pAnimator->CopyPhysics();
      }
      break;
    case ID_MAINMENU_RAGDOLL_PASTE:
      {
        g_pAnimator->PastePhysics();
      }
      break;
    case ID_MAINMENU_RAGDOLL_ADDPARTICLE:
      {
        g_pAnimator->AddParticle();
      }
      break;
    case ID_MAINMENU_RAGDOLL_ENABLECOLLISION:
      {
        g_pAnimator->ToggleParticleCollision(true);
      }
      break;
    case ID_MAINMENU_RAGDOLL_DISABLECOLLISION:
      {
        g_pAnimator->ToggleParticleCollision(false);
      }
      break;
    case ID_MAINMENU_RAGDOLL_DELETEPARTICLE:
      {
        g_pAnimator->RemoveParticle();
      }
      break;
    case ID_MAINMENU_RAGDOLL_SETCONNECTDISTANCE:
      {
        g_pAnimator->SetConnectDistance();
      }
      break;
    case ID_MAINMENU_RAGDOLL_CONNECT:
      {
        g_pAnimator->ConnectParticles(true);
      }
      break;
    case ID_MAINMENU_RAGDOLL_UNCONNECT:
      {
        g_pAnimator->ConnectParticles(false);
      }
      break;
    case ID_MAINMENU_RAGDOLL_MODE:
      {
        g_pAnimator->TogglePhysicsMode();
      }
      break;
    case ID_MAINMENU_RAGDOLL_PREVIEW:
      {
        g_pAnimator->TogglePhysicsPreviewMode();
      }
      break;
    case ID_MAINMENU_RAGDOLL_SETPARENT:
      {
        g_pAnimator->SetParticleParent();
      }
      break;
    case ID_MAINMENU_RAGDOLL_SETMOVEPOINT:
      {
        g_pAnimator->SetParticleAsMovePoint(true);
      }
      break;
    case ID_MAINMENU_RAGDOLL_UNSETMOVEPOINT:
      {
        g_pAnimator->SetParticleAsMovePoint(false);
      }
      break;
    case ID_MAINMENU_RAGDOLL_REMMOVEPOINT:
      {
        g_pAnimator->RemoveMovePointsFromSprite();
      }
      break;
    case ID_MAINMENU_VIEW_BGCOLOR:
      {
        if(bgcolor == 0)
          bgcolor = 128;
        else if(bgcolor == 128)
          bgcolor = 255;
        else if(bgcolor == 255)
          bgcolor = 0;
      }
      break;
    case ID_MAINMENU_VIEW_GRID:
      {
        g_pAnimator->ToggleGrid();
      }
      break;
  }
}

long initapp (long argc, char **argv)
{
  printf("Loading animator...");

  xres = 640; yres = 480; colbits = 32; fullscreen = 0;
  long cpuoption = -1;
	for(int i=argc-1;i>0;i--)
	{
		//if ((argv[i][0] != '/') && (argv[i][0] != '-')) { argfilindex = i; continue; }
		if (!stricmp(&argv[i][1],"fullscreen")) { fullscreen = 1; continue; }
		if (!stricmp(&argv[i][1],"3dn")) { cpuoption = 0; continue; }
		if (!stricmp(&argv[i][1],"sse")) { cpuoption = 1; continue; }
		if (!stricmp(&argv[i][1],"sse2")) { cpuoption = 2; continue; }
		//if (!stricmp(&argv[i][1],"?")) { showinfo(); return(-1); }
		if ((argv[i][1] >= '0') && (argv[i][1] <= '9'))
		{
			int k = 0; int z = 0;
			for(int j=1;;j++)
			{
				if ((argv[i][j] >= '0') && (argv[i][j] <= '9'))
					{ k = (k*10+argv[i][j]-48); continue; }
				switch (z)
				{
					case 0: xres = k; break;
					case 1: yres = k; break;
					//case 2: colbits = k; break;
				}
				if (!argv[i][j]) break;
				z++; if (z > 2) break;
				k = 0;
			}
		}
	}
	if (xres > MAXXDIM) xres = MAXXDIM;
	if (yres > MAXYDIM) yres = MAXYDIM;
	extern long cputype;
	switch(cpuoption)
	{
		case 0: cputype &= ~((1<<25)|(1<<26)); cputype |= ((1<<30)|(1<<31)); break;
		case 1: cputype |= (1<<25); cputype &= ~(1<<26); cputype &= ~((1<<30)|(1<<31)); break;
		case 2: cputype |= ((1<<25)|(1<<26)); cputype &= ~((1<<30)|(1<<31)); break;
		default:;
	}

  if(initvoxlap() < 0) return -1;

  vx5.kv6mipfactor = 1536;
  vx5.curcol = 0;
	vx5.lightmode = 0;
	vx5.vxlmipuse = 9;
  vx5.mipscandist = 192;
	vx5.fallcheck = 0;
	vx5.maxscandist = (long)(VSID*1.42);
	//updatevxl();

  NONCLIENTMETRICS ncm;
  ZeroMemory(&ncm, sizeof(NONCLIENTMETRICS));
  ncm.cbSize = sizeof(NONCLIENTMETRICS);
  SystemParametersInfo(SPI_GETNONCLIENTMETRICS, NULL, &ncm, NULL);
  g_hFont = CreateFontIndirect(&ncm.lfMessageFont);

  MenuFunc = OnMenuClick;
  catchwmclose = OnClose;
  g_pAnimator = new Animator();
  return(0);
}

void handlemouse()
{
  // read mouse state
  float fmousx, fmousy;
  long bstatus;
  readmouse(&fmousx, &fmousy, &bstatus);
  if(Config::Input_bMouseInverted)
    fmousy = -fmousy;

  fmousx *= Config::Input_fMouseSensitivityX;
  fmousy *= Config::Input_fMouseSensitivityY;
  
  g_pAnimator->SetMouse(fmousx, fmousy, bstatus & 1 ? true : false, bstatus & 2 ? true : false);
}

void initapp2()
{
  // get window size
  int sizex,sizey;
  RECT rect;
  GetClientRect(ghwnd, &rect);
  sizex = rect.right;
  sizey = rect.bottom;

  // load menu
  HMENU menu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(ID_MAINMENU));
  SetMenu(ghwnd, menu);

  // resize window until size is correct
  GetClientRect(ghwnd, &rect);
  int x = sizex;
  int y = sizey;
  while(rect.bottom != sizey || rect.right != sizex)
  {
    SetWindowPos(ghwnd, 0, 0, 0, x, y, SWP_NOZORDER|SWP_NOMOVE);
    GetClientRect(ghwnd, &rect);
    if(sizex != rect.right)
      x -= rect.right - sizex;
    if(sizey != rect.bottom)
      y -= rect.bottom - sizey;
  }

  // create windows
  g_pAnimator->CreateWindows();
  printf("done.\n");
}

void drawsprites()
{
  // render ordinary sprites
  for(unsigned int i = 0; i < g_Sprites.size(); i++)
  {
    drawsprite(g_Sprites[i]);
  }
}
void doframe ()
{
  static double lastTime = Time::GetTime();
  double time = Time::GetTime();
  double deltaTime = time - lastTime;
  lastTime = time;

  // safety clamp
  if(deltaTime > 0.1f)
    deltaTime = 0.1f;

  handlemouse();

  // update camera
  Matrix44 mCamera = ConvertToVoxlap(g_pAnimator->GetCameraMatrix());
  ipos = MakePoint<dpoint3d>(mCamera.GetTranslation());
  istr = MakePoint<dpoint3d>(mCamera.GetSideVector());
  ihei = MakePoint<dpoint3d>(-mCamera.GetUpVector()); // this is down vector
  ifor = MakePoint<dpoint3d>(mCamera.GetFrontVector());

  // render
  long frameptr, pitch, xdim, ydim;
  startdirectdraw(&frameptr,&pitch,&xdim,&ydim);
  voxsetframebuffer(frameptr,pitch,xdim,ydim);
  setcamera(&ipos,&istr,&ihei,&ifor,xdim*.5,ydim*.5,xdim*.5);
  //setears3d(ipos.x,ipos.y,ipos.z,ifor.x,ifor.y,ifor.z,ihei.x,ihei.y,ihei.z);
  //opticast();
  clearscreen(true, true, bgcolor);

  // update animator
  g_pAnimator->Run(deltaTime);

  // render debug lines
  for(unsigned int i = 0; i < g_DebugLines.size();)
  {
    drawline3d(g_DebugLines[i].vStart.x, g_DebugLines[i].vStart.y, g_DebugLines[i].vStart.z,
      g_DebugLines[i].vEnd.x, g_DebugLines[i].vEnd.y, g_DebugLines[i].vEnd.z,
      (0xFF<<24) | ((g_DebugLines[i].iR&0xFF)<<16) | ((g_DebugLines[i].iG&0xFF)<<8) | (g_DebugLines[i].iB&0xFF));
    if(!g_DebugLines[i].bPersistent)
    {
      g_DebugLines[i] = g_DebugLines.back();
      g_DebugLines.pop_back();
    }
    else
    {
      i++;
    }
  }

  // status text
  if(g_pAnimator->GetAnimation()->IsRagdoll())
  {
    print6x8(10,10,0xFFFFFF,-1,"Press ESC to stop...");
  }
  else if(g_pAnimator->IsInSpritePickingMode())
  {
    print6x8(10,10,0xFFFFFF,-1,"Pick a sprite...");
    print6x8(10,30,0xFF0000,-1,"Parented");
    print6x8(10,40,0x00FFFF,-1,"Movepoint");
    print6x8(10,50,0xFFFF00,-1,"Parented movepoint");
  }

  stopdirectdraw();
  nextpage();
  readkeyboard();
}

void uninitapp ()
{
  delete g_pAnimator;
  DeleteObject(g_hFont);
  kzuninit();
}

