#include "StdAfx.h"
#include "Voxlap/msvc.h"
#include "Voxlap/sysmain.h"
#include "Voxlap/voxlap5.h"
#include "Engine/VoxlapConversions.h"
#include "Engine/SXLParser.h"
#include "Engine/Sound/SoundDevice.h"
#include "Game/ParticleSystems/ParticleSystem.h"
#include "Game/HUD.h"
#include "Game/GameMenu.h"

#define DIRECTINPUT_VERSION 0x0800
#include "dinput.h"

int numsprites = 0;
struct spritetype : vx5sprite //Note: C++!
{
  point3d v, r;  //other attributes (not used by voxlap engine)
  long owner, tim, tag;
};
static const int MAXSPRITES = 1024;
spritetype spr[MAXSPRITES];
dpoint3d ipos, istr, ihei, ifor;

// color filter
extern int g_iColorFilterR;
extern int g_iColorFilterG;
extern int g_iColorFilterB;
extern float g_fColorFilterTime;

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
std::vector<vx5sprite *> g_RenderInFrontSprites;

void RegisterSprite(vx5sprite *sprite, bool bRenderInFront)
{
  std::vector<vx5sprite *> &Sprites = bRenderInFront ? g_RenderInFrontSprites : g_Sprites;
  Sprites.push_back(sprite);
}

void UnregisterSprite(vx5sprite *sprite, bool bRenderInFront)
{
  std::vector<vx5sprite *> &Sprites = bRenderInFront ? g_RenderInFrontSprites : g_Sprites;
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

// particles
std::vector<ParticleSystem *> g_Particles;
void RegisterParticleSystem(ParticleSystem *pSystem)
{
  g_Particles.push_back(pSystem);
}

void UnregisterParticleSystem(ParticleSystem *pSystem)
{
  for(unsigned int i = 0; i < g_Particles.size(); i++)
  {
    if(g_Particles[i] == pSystem)
    {
      // replace with last
      g_Particles[i] = g_Particles.back();
      g_Particles.pop_back();
      break;
    }
  }
}


Game *g_pGame = NULL;


void load_vxl_and_sprites(void)
{
  char *vxlnam, *skynam, *kv6nam, *userst;
  kv6data *tempkv6;

	if (loadsxl("vxl/default.sxl",&vxlnam,&skynam,&userst))
	{  //.SXL valid so load sprite info out of .SXL

		if (!loadvxl(vxlnam,&ipos,&istr,&ihei,&ifor))
			loadnul(&ipos,&istr,&ihei,&ifor);
		loadsky(skynam);

    g_pGame = new Game();

		for(numsprites=0;kv6nam=parspr(&spr[numsprites],&userst);numsprites++)
		{
			if (numsprites >= MAXSPRITES) continue; //OOPS! Just to be safe

		  //KLIGHT is dummy sprite for light sources - don't load it!
			if(!stricmp(kv6nam,"KV6\\KLIGHT.KV6"))
			{
				//Copy light position info here!
				continue;
			}

      // parse other placeholders
      bool bKeepSprite = true;
      GetSXLParserManager()->Parse(kv6nam, &spr[numsprites], bKeepSprite);
      if(!bKeepSprite)
        continue;

			getspr(&spr[numsprites],kv6nam);

		  //Generate all lower mip-maps here:
			if (!(spr[numsprites].flags&2)) //generate mips for KV6 in SXL
			{
				tempkv6 = spr[numsprites].voxnum;
				while (tempkv6 = genmipkv6(tempkv6));
			}
			else //generate mips for KFA in SXL
			{
				for(int i=(spr[numsprites].kfaptr->numspr)-1;i>=0;i--)
				{
					tempkv6 = spr[numsprites].kfaptr->spr[i].voxnum;
					while (tempkv6 = genmipkv6(tempkv6));
				}
			}
		}
  }
  else
  {
    extern void evilquit (const char *);
    evilquit("Game data not found!");
  }
}

long initapp (long argc, char **argv)
{
  xres = 640; yres = 480; colbits = 32; fullscreen = 1;
  Config::Load();
  long cpuoption = -1;
	for(int i=argc-1;i>0;i--)
	{
		//if ((argv[i][0] != '/') && (argv[i][0] != '-')) { argfilindex = i; continue; }
		if (!stricmp(&argv[i][1],"win")) { fullscreen = 0; continue; }
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
  kzaddstack("voxdata.zip");
  kzaddstack("wolfsw.zip");

  load_vxl_and_sprites();

  vx5.kv6mipfactor = 128;
  vx5.curcol = 0;
	vx5.lightmode = 0;
	vx5.vxlmipuse = 9;
  vx5.mipscandist = 192;
	vx5.fallcheck = 1;
	updatevxl();

	vx5.maxscandist = (long)(VSID*1.42);

  g_pGame->CreatePlayer(ConvertFromVoxlap(ipos));

  extern unsigned long calcglobalmass ();
  g_pGame->GetStats().iVoxels = calcglobalmass();

  void *p=malloc(1);
  free(p);

  g_pGameMenu = new GameMenu();
  // fade in from menu
  Voxlap::SetColorFilter(0,0,0, 1.0f);
  return(0);
}

SmartPointer<Sound> g_pMusic = NULL;
void initapp2()
{
  g_pSoundDevice = new SoundDevice();
  g_pSoundDevice->Create(SoundDevice::DeviceSettings());
  g_pSoundManager = new SoundManager();

  // start music
  g_pMusic = GetSoundManager()->CreateStreamingSound("Wolf3D:mus77325");
  g_pMusic->SetLooping(true);
  g_pMusic->Play();
}

static char old_keystatus[256];

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
  
  g_pGame->SetMouse(fmousx, fmousy);
}

void handlekeyb()
{
  // read key states
  readkeyboard();
  g_pGame->SetKey(Game::Key_Forward, keystatus[Config::Keybind_Forward] ? true : false);
  g_pGame->SetKey(Game::Key_Backward, keystatus[Config::Keybind_Backward] ? true : false);
  g_pGame->SetKey(Game::Key_Left, keystatus[Config::Keybind_Left] ? true : false);
  g_pGame->SetKey(Game::Key_Right, keystatus[Config::Keybind_Right] ? true : false);
  g_pGame->SetKey(Game::Key_Jump, keystatus[Config::Keybind_Jump] ? true : false);
  g_pGame->SetKey(Game::Key_Fire, ext_mbstatus[0]&1 ? true : false);
  g_pGame->SetKey(Game::Key_Use, keystatus[Config::Keybind_Use] ? true : false);
  g_pGame->SetKey(Game::Key_Crouch, keystatus[Config::Keybind_Crouch1] || keystatus[Config::Keybind_Crouch2] ? true : false);
  g_pGame->SetKey(Game::Key_Flashlight, keystatus[Config::Keybind_Flashlight] ? true : false);

  // num keys
  for(int i = 0; i < 9; i++)
  {
    g_pGame->SetKey(Game::Key_Weapon1 + i, keystatus[DIK_1 + i] ? true : false);
  }

  // mouse invert
  if(keystatus[DIK_Y] && !old_keystatus[Config::Keybind_MouseInvert]) Config::Input_bMouseInverted = !Config::Input_bMouseInverted;

  // remember keys
  memcpy(old_keystatus, keystatus, 256*sizeof(char));
}

void doframe ()
{
  static double fLastTime = Time::GetTime();
  double fTime = Time::GetTime();
  double fDeltaTime = fTime - fLastTime;
  fLastTime = fTime;

  // update sounds
  GetSoundManager()->Update();
  GetSoundDevice()->Update();

  if(GetGameMenu()->IsOpen())
  {   
    // run menu
    readkeyboard();
    float fmousx, fmousy;
    long bstatus;
    readmouse(&fmousx, &fmousy, &bstatus);
    long frameptr, pitch, xdim, ydim;
    startdirectdraw(&frameptr,&pitch,&xdim,&ydim);
    voxsetframebuffer(frameptr,pitch,xdim,ydim);
    clearscreen(true, true);
    GetGameMenu()->Run(fDeltaTime);
    stopdirectdraw();
    nextpage();
    return;
  }

  // handle input
  handlemouse();
  handlekeyb();

  // run game
  g_pGame->Run();

  // flashlight effect
  if(g_pGame->IsFlashLightEnabled())
  {
    Vector3D vPos = ConvertToVoxlap<Vector3D>(g_pGame->GetFlashLightPosition());
	  voxbackup(vPos.x-64,vPos.y-64,vPos.x+65,vPos.y+65,0x20000);
	  setnormflash(vPos.x,vPos.y,vPos.z,64,5000 * g_pGame->GetFlashLightIntensity());
  }

  // update camera
  Matrix44 mCamera = ConvertToVoxlap(g_pGame->GetCameraMatrix());
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
  opticast();

  // flashlight effect
  if(g_pGame->IsFlashLightEnabled())
  {
    voxrestore();
  }

  // handle falling of blown pieces
  void handlefalls();
  handlefalls();

  // render ordinary sprites
  for(unsigned int i = 0; i < g_Sprites.size(); i++)
  {
    drawsprite(g_Sprites[i]);
  }

  // render particles
  for(unsigned int i = 0; i < g_Particles.size(); i++)
  {
    g_Particles[i]->Render(fDeltaTime);
  }

  // render map sprites
  for(int i = 0; i < numsprites; i++)
  {
		if(spr[i].flags&2)
			animsprite(&spr[i],fDeltaTime*1000.0);
		drawsprite(&spr[i]);
	}

  // clear z buffer
  clearscreen(false, true);

  // render front sprites
  for(unsigned int i = 0; i < g_RenderInFrontSprites.size(); i++)
  {
    drawsprite(g_RenderInFrontSprites[i]);
  }

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

  // apply color filter
  if(g_fColorFilterTime > 0)
  {
    int iFactor = Max(1.0f - g_fColorFilterTime, 0.0f) * 255;
    int iR = Min(g_iColorFilterR + iFactor, 256);
    int iG = Min(g_iColorFilterG + iFactor, 256);
    int iB = Min(g_iColorFilterB + iFactor, 256);

    applycolorfilter(iR,iG,iB);

    g_fColorFilterTime -= fDeltaTime;
    if(g_fColorFilterTime < 0)
      g_fColorFilterTime = 0;
  }

  if(GetGame()->IsLevelFinished())
  {
    g_pMusic->Stop();
    g_pMusic = GetSoundManager()->CreateStreamingSound("Wolf3D:mus109215");
    g_pMusic->SetLooping(true);
    g_pMusic->Play();
    GetGameMenu()->OpenFinishScreen();
  }

  // draw hud
  DrawHUD(fDeltaTime);

  // show fps
  if(Config::Debug_bShowFPS)
  {
    static double lastFPSUpdate = 0;
    static int fps = 0;
    static int fps_frames = 0;
    lastFPSUpdate += fDeltaTime;
    fps_frames++;
    if(lastFPSUpdate > 0.5f)
    {
      fps = fps_frames / lastFPSUpdate;
      fps_frames = 0;
      lastFPSUpdate = 0;
    }
    print4x6(0,0,0xc0c0c0,-1,"FPS %i",fps);
  }

  stopdirectdraw();
  nextpage();
  readkeyboard();
  if(keystatus[1])
    quitloop();
}

void uninitapp ()
{
  delete g_pGameMenu;
  delete g_pGame;
  g_pMusic = NULL;
  delete g_pSoundManager;
  g_pSoundDevice->Destroy();
  delete g_pSoundDevice;
  uninitvoxlap();
  kzuninit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Following stuff is copy & pasted from Ken's example game...

void explodesprite (vx5sprite *spr, float vel, long liquid, long stepsize)
{
	point3d fp, fp2, fp3, fp4;
	kv6voxtype *v, *ve;
	kv6data *kv;
	long i, x, y, z;

		//WARNING: This code will change if I re-design the KV6 format!
	kv = spr->voxnum; if (!kv) return;
	v = kv->vox;
	i = -(rand()%stepsize); //i = stepstart;
	stepsize--;
	for(x=0;x<kv->xsiz;x++)
	{
		fp.x = x - kv->xpiv;
		fp2.x = spr->s.x*fp.x + spr->p.x;
		fp2.y = spr->s.y*fp.x + spr->p.y;
		fp2.z = spr->s.z*fp.x + spr->p.z;
		for(y=0;y<kv->ysiz;y++)
		{
			fp.y = y - kv->ypiv;
			fp3.x = spr->h.x*fp.y + fp2.x;
			fp3.y = spr->h.y*fp.y + fp2.y;
			fp3.z = spr->h.z*fp.y + fp2.z;
			for(ve=&v[kv->ylen[x*kv->ysiz+y]];v<ve;v++)
			{
#if 1          //Surface voxels only
				i--; if (i >= 0) continue; i = stepsize;
				fp.z = v->z - kv->zpiv;
				fp4.x = spr->f.x*fp.z + fp3.x;
				fp4.y = spr->f.y*fp.z + fp3.y;
				fp4.z = spr->f.z*fp.z + fp3.z;
				//spawndebris(&fp4,vel,v->col,1,liquid);
#else          //All voxels in volume. WARNING: use only for very small KV6!
				if (v->vis&16) z = v->z;
				for(;z<=v->z;z++)
				{
					i++; if (i < 0) continue; i = stepsize;
					fp.z = z - kv->zpiv;
					fp4.x = spr->f.x*fp.z + fp3.x;
					fp4.y = spr->f.y*fp.z + fp3.y;
					fp4.z = spr->f.z*fp.z + fp3.z;
					//spawndebris(&fp4,vel,v->col,1,liquid);
				}
#endif
			}
		}
	}
}

void deletesprite (long index)
{
	numsprites--;
	//playsoundupdate(&spr[index].p,(point3d *)0);
	//playsoundupdate(&spr[numsprites].p,&spr[index].p);
	spr[index] = spr[numsprites];
}

void vecrand (float sc, point3d *a)
{
	float f;

		//UNIFORM spherical randomization (see spherand.c)
	a->z = ((double)(rand()&32767))/16383.5-1.0;
	f = (((double)(rand()&32767))/16383.5-1.0)*PI; a->x = cos(f); a->y = sin(f);
	f = sqrt(1.0 - a->z*a->z)*sc; a->x *= f; a->y *= f; a->z *= sc;
}

void handlefalls()
{
  long j,i,k,totclk;
  static double dtotclk = 0;

  static char unitfalldelay[255] =
  {
    125,51,40,34,29,27,24,23,22,20,19,19,17,17,17,16,
	  15,15,14,15,13,14,13,13,13,12,12,12,12,11,11,12,
	  11,10,11,11,10,10,10,10,10,10,9,10,9,9,9,10,9,8,9,9,9,8,9,8,8,8,9,8,8,8,8,8,
	  7,8,8,7,8,7,8,7,8,7,7,7,7,7,8,7,7,6,7,7,7,7,6,7,7,6,7,6,7,6,7,6,
	  7,6,6,7,6,6,6,6,6,6,7,6,6,6,5,6,6,6,6,6,6,5,6,6,6,5,6,5,6,6,5,6,
	  5,6,5,6,5,5,6,5,6,5,5,6,5,5,5,6,5,5,5,5,5,5,6,5,5,5,5,5,5,5,5,5,
	  5,4,5,5,5,5,5,5,5,4,5,5,5,4,5,5,5,4,5,5,4,5,4,5,5,4,5,4,5,5,4,5,
	  4,5,4,5,4,4,5,4,5,4,4,5,4,5,4,4,5,4,4,5,4,4,4,5,4,4,4,5,4,4,4,4,
	  5,4,4,4,4,4,4,4,5,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
  };
  #define DEBRIS "wav/debris.wav"
  #define DEBRIS2 "wav/plop.wav"
  #define DIVEBORD "wav/divebord.wav"

	static double odtotclk = 0;
  odtotclk = dtotclk;
  readklock(&dtotclk);
	totclk = (long)(dtotclk*1000.0);
  float fsynctics = (float)(dtotclk-odtotclk);

	updatevxl();
	do
	{
		j = 0;
		startfalls();
		for(i=vx5.flstnum-1;i>=0;i--)
		{
			if (vx5.flstcnt[i].userval == -1) //New piece
			{
				vx5.flstcnt[i].userval2 = totclk+unitfalldelay[0];
				vx5.flstcnt[i].userval = 1;

					//New piece!
				//fp = vx5.flstcnt[i].centroid;
				if ((numsprites < MAXSPRITES) && (meltfall(&spr[numsprites],i,1)))
				{
					k = numsprites++;
					//playsound(DEBRIS,vx5.flstcnt[i].mass>>4,(float)(rand()-16384)*.00001+0.5,&spr[k].p,KSND_MOVE);

					spr[k].tag = -17; spr[k].tim = totclk; spr[k].owner = 0;
					spr[k].v.x = 0;
					spr[k].v.y = 0;
					spr[k].v.z = 0;

				  spr[k].r.x = 0;
					spr[k].r.y = 0;
					spr[k].r.z = 0;
					continue;
				}
			}
			if (totclk >= vx5.flstcnt[i].userval2) //Timeout: drop piece #i by 1 unit
			{
				//dofall(i);
				vx5.flstcnt[i].userval2 += unitfalldelay[vx5.flstcnt[i].userval++];
				if (totclk >= vx5.flstcnt[i].userval2) j = 1;
			}
		}
		finishfalls();
	} while (j);

  float f;
  point3d fp, fpos;
  lpoint3d lp;

	for(i=numsprites-1;i>=0;i--)
	{
		if (spr[i].tag == -17) //Animate melted sprites
		{
			if ((totclk > spr[i].tim+2000) || (spr[i].owner >= 3))
			{
				//spr[i].s.x *= .97; spr[i].s.y *= .97; spr[i].s.z *= .97;
				//spr[i].h.x *= .97; spr[i].h.y *= .97; spr[i].h.z *= .97;
				//spr[i].f.x *= .97; spr[i].f.y *= .97; spr[i].f.z *= .97;
				//if (spr[i].s.x*spr[i].s.x + spr[i].s.y*spr[i].s.y + spr[i].s.z*spr[i].s.z < .1*.1)
				{
					if (spr[i].voxnum)
					{
						j = spr[i].voxnum->vox[0].col;

						//playsound(DEBRIS2,70,(float)(rand()-32768)*.00002+1.0,&spr[i].p,KSND_3D);
						explodesprite(&spr[i],.125,0,3);

						//spawndebris(&spr[i].p,1,j,16,0);
						//spawndebris(&spr[i].p,0.5,j,8,0);
						//spawndebris(&spr[i].p,0.25,j,4,0);

						//setkv6(&spr[i],0);

							//Delete temporary sprite data from memory
						free(spr[i].voxnum);
					}
					deletesprite(i);
					continue;
				}
			}

			fpos = spr[i].p;

				//Do velocity & gravity
			spr[i].v.z += fsynctics*64;
			spr[i].p.x += spr[i].v.x * fsynctics;
			spr[i].p.y += spr[i].v.y * fsynctics;
			spr[i].p.z += spr[i].v.z * fsynctics;
			spr[i].v.z += fsynctics*64;

				//Do rotation
			f = min(totclk-spr[i].tim,250)*fsynctics*.01;
			axisrotate(&spr[i].s,&spr[i].r,f);
			axisrotate(&spr[i].h,&spr[i].r,f);
			axisrotate(&spr[i].f,&spr[i].r,f);

				//Make it bounce
			if (!cansee(&fpos,&spr[i].p,&lp))  //Wake up immediately if it hit a wall
			{
				spr[i].p = fpos;
				estnorm(lp.x,lp.y,lp.z,&fp);
				f = (spr[i].v.x*fp.x + spr[i].v.y*fp.y + spr[i].v.z*fp.z)*2.f;
				spr[i].v.x = (spr[i].v.x - fp.x*f)*.75f;
				spr[i].v.y = (spr[i].v.y - fp.y*f)*.75f;
				spr[i].v.z = (spr[i].v.z - fp.z*f)*.75f;
				vecrand(1.0,&spr[i].r);
				//spr[i].r.x = ((float)rand()/16383.5f)-1.f;
				//spr[i].r.y = ((float)rand()/16383.5f)-1.f;
				//spr[i].r.z = ((float)rand()/16383.5f)-1.f;

				if (f > 96)
				{
						//Make it shatter immediately if it hits ground too quickly
					if (spr[i].voxnum)
					{
						j = spr[i].voxnum->vox[0].col;

						//playsound(DEBRIS2,70,(float)(rand()-32768)*.00002+1.0,&spr[i].p,KSND_3D);
						explodesprite(&spr[i],.125,0,3);

						//spawndebris(&spr[i].p,f*.5  /96,j,16,0);
						//spawndebris(&spr[i].p,f*.25 /96,j,8,0);
						//spawndebris(&spr[i].p,f*.125/96,j,4,0);

							//Delete temporary sprite data from memory
						free(spr[i].voxnum);
					}
					deletesprite(i);
					continue;
				}
				else
				{
					//playsound(DIVEBORD,25,(float)(rand()-16384)*.00001+1.0,&spr[i].p,KSND_3D);
					spr[i].owner++;
				}
			}
		}
  }
}
