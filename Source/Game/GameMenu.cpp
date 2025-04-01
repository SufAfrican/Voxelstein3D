#include "StdAfx.h"
#include "Game/GameMenu.h"
#include "Voxlap/msvc.h"
#include "Voxlap/voxlap5.h"
#include "Engine/SystemInfo.h"

class GameMenuScreen
{
public:
  GameMenuScreen() {}
  virtual ~GameMenuScreen() {}
  virtual void Run(float fDeltaTime) {}
};

/// Simple fading screen that pops itself out of the stack
class IntroScreen : public GameMenuScreen
{
public:
  float m_fTime;
  VoxlapImage *m_pScreen;

  enum State
  {
    State_FadingIn,
    State_Normal,
    State_FadingOut
  };
  State m_CurrentState;

public:
  IntroScreen(const char *strScreen)
  {
    m_fTime = 0.0f;
    m_pScreen = Voxlap::LoadImage(strScreen);
    m_CurrentState = State_FadingIn;
  }

  virtual void OnEnd(void)
  {
    GetGameMenu()->m_ScreenStack.pop_back();
    delete this;
  }

  virtual bool KeyDetect(void)
  {
    return GetGameMenu()->PressedAnyKey();
  }

  void Run(float fDeltaTime)
  {
    // fade in
    if(m_CurrentState == State_FadingIn)
    {
      m_fTime += fDeltaTime;
      GetGameMenu()->m_iBrightness = Min(m_fTime * 3.0f, 1.0f) * 255;
      if(GetGameMenu()->m_iBrightness == 255)
      {
        m_CurrentState = State_Normal;
      }
    }
    // fade out
    else if(m_CurrentState == State_FadingOut)
    {
      m_fTime -= fDeltaTime;
      if(m_fTime < 0.0f)
      {
        OnEnd();
        return;
      }
      GetGameMenu()->m_iBrightness = Clamp(m_fTime * 6.0f, 0.0f, 1.0f) * 255;
    }

    // fade out
    if(m_CurrentState == State_Normal)
    {
      if(KeyDetect())
      {
        // start fading out
        m_CurrentState = State_FadingOut;
        m_fTime = 0.5f;
      }
    }

    // draw screen
    if(m_pScreen != NULL)
    {
      int iScreenSizeX, iScreenSizeY;
      getscreensize(iScreenSizeX, iScreenSizeY);
      int iTileScaleX = 65536 * iScreenSizeX / 640;
      int iTileScaleY = 65536* iScreenSizeY / 480;

      drawtile(m_pScreen->pData,
                m_pScreen->iBytesPerLine,
                m_pScreen->iWidth,
                m_pScreen->iHeight,
                0<<16, 0<<16,
                0<<16, 0<<16, iTileScaleX, iTileScaleY, 0, -1);
    }
  }

};

/// Intro screen with system info
class IntroStatsScreen : public IntroScreen
{
public:
  unsigned int m_iCPUSpeed;
  unsigned int m_iCPUs;

public:
  IntroStatsScreen() : IntroScreen("png/startscreen.png")
  {
    m_iCPUSpeed = GetSystemInfo()->GetCPUSpeed(0);
    m_iCPUs = GetSystemInfo()->GetNumCPUs();
  }

  void Run(float fDeltaTime)
  {
    IntroScreen::Run(fDeltaTime);

    // mmx
    GetGameMenu()->DrawTickBox(GetSystemInfo()->HasMMX(), 383, 178);    
    // 3dnow
    GetGameMenu()->DrawTickBox(GetSystemInfo()->Has3DNow(), 383, 178+76*1);    
    // sse
    GetGameMenu()->DrawTickBox(GetSystemInfo()->HasSSE(), 383, 178+76*2);    
    // sse2
    GetGameMenu()->DrawTickBox(GetSystemInfo()->HasSSE2(), 383, 178+76*3);    

    // memory  & cpu speed
    {
      unsigned int iTotalMem, iFreeMem;
      GetSystemInfo()->GetMemory(iTotalMem, iFreeMem);
      static const int iValue[10] = {4000,3500,3000,2500,2000,1500,1000,700,500,200};
      for(int i = 0; i < 10; i++)
      {
        GetGameMenu()->DrawTickBox(iTotalMem >= iValue[i], 106, 185 + 22 * i, 50);
        GetGameMenu()->DrawTickBox(m_iCPUSpeed >= iValue[i], 219, 185 + 22 * i, 50);
      }
    }

    // number of CPUs
    {
      int pos = 0;
      for(int i = 10; i >= 1; i--)
      {
        GetGameMenu()->DrawTickBox(m_iCPUs >= i, 306, 185 + 22 * pos, 50);
        pos++;
      }
    }
  }
};

GameMenu *g_pGameMenu = NULL;

GameMenu::GameMenu()
{
  m_pBoxTicked = Voxlap::LoadImage("png/box_ticked.png");
  m_pBoxUnticked = Voxlap::LoadImage("png/box_unticked.png");
  m_iBrightness = 0;

  if(Config::Debug_bShowIntro)
  {
    m_ScreenStack.push_back(new IntroScreen("png/intro.png"));
    m_ScreenStack.push_back(new IntroScreen("png/pg-18.png"));
    m_ScreenStack.push_back(new IntroStatsScreen());
  }
}

GameMenu::~GameMenu()
{
  for(unsigned int i = 0; i < m_ScreenStack.size(); i++)
    delete m_ScreenStack[i];
}

bool GameMenu::PressedAnyKey(void)
{
  extern char keystatus[256];
  for(int i = 0; i < 256; i++)
  {
    if(keystatus[i])
    {
      return true;
    }
  }
  return false;
}

void GameMenu::DrawTickBox(bool bTicked, int iX, int iY, int iSize)
{
  int iScreenSizeX, iScreenSizeY;
  getscreensize(iScreenSizeX, iScreenSizeY);
  iX = (iX * iScreenSizeX) / 640;
  iY = (iY * iScreenSizeY) / 480;

  int iScaleX = 65536 * iScreenSizeX / 640 * iSize / 100;
  int iScaleY = 65536 * iScreenSizeY / 480 * iSize / 100;

  VoxlapImage *pImg = bTicked ? m_pBoxTicked : m_pBoxUnticked;
  drawtile(pImg->pData,
            pImg->iBytesPerLine,
            pImg->iWidth,
            pImg->iHeight,
            0<<16, 0<<16,
            iX<<16, iY<<16, iScaleX, iScaleY, 0, -1);
}

void GameMenu::Run(float fDeltaTime)
{
  if(!m_ScreenStack.empty())
  {
    GameMenuScreen *pScreen = m_ScreenStack.back();
    pScreen->Run(fDeltaTime);
  }

  applycolorfilter(m_iBrightness,m_iBrightness,m_iBrightness);
}

#include "Game/Entities/PlayerEntity.h"

/// End of game screen
class FinishScreen : public IntroScreen
{
public:
  struct Counter
  {
    Counter() {}
    Counter(unsigned int iTarget, unsigned iMax) : iValue(0), iTarget(iTarget), iMax(iMax) {}
    unsigned int iValue;
    unsigned int iTarget;
    unsigned int iMax;
  };
  int m_iCurrentCounter;
  Counter m_Counters[4];
  SmartPointer<Sound> m_pCountSound;
  float m_fLastCountTime;

  VoxlapImage *m_pStatNumbers;

public:
  FinishScreen() : IntroScreen("png/endscreen.png")
  {
    extern unsigned long calcglobalmass ();
    m_iCurrentCounter = 0;
    m_fLastCountTime = 0.0f;
    m_Counters[0] = Counter(GetGame()->GetLocalPlayer()->GetStats().iKills, GetGame()->GetStats().iEnemies);
    m_Counters[1] = Counter(GetGame()->GetLocalPlayer()->GetStats().iItems, GetGame()->GetStats().iItems);
    m_Counters[2] = Counter(GetGame()->GetLocalPlayer()->GetStats().iTreasures, GetGame()->GetStats().iTreasures);
    m_Counters[3] = Counter(GetGame()->GetStats().iVoxels - calcglobalmass(), GetGame()->GetStats().iVoxels);
    m_pStatNumbers = Voxlap::LoadImage("png/statnumbers.png");
    m_pCountSound = GetSoundManager()->CreateSound2D("Wolf3D:633857,3593,0,3593");

    Sound *pSound = GetSoundManager()->CreateSound2D("Wolf3D:735287,6805,1411133,6805");
    pSound->Play();
  }

  virtual void OnEnd(void)
  {
    IntroScreen::OnEnd();
    extern void quitloop();
    quitloop();
  }

  virtual bool KeyDetect(void)
  {
    extern char keystatus[256];
    return keystatus[1] ? true : false;
  }

  void DrawStatNumber(int iNum, int iMax, int iX, int iY)
  {
    int iScreenSizeX, iScreenSizeY;
    getscreensize(iScreenSizeX, iScreenSizeY);
    int iTileScaleX = 65536 * iScreenSizeX / 640;
    int iTileScaleY = 65536 * iScreenSizeY / 480;

    iX = (iX * iScreenSizeX) / 640;
    iY = (iY * iScreenSizeY) / 480;

    char strNum[128];
    sprintf(strNum,"%i/%i",iNum, iMax);
    for(int i = 0; i < 128; i++)
    {
      if(strNum[i] == 0)
        return;

      if((strNum[i] < '0' || strNum[i] > '9') && strNum[i] != '/')
        continue;

      int iDigit = strNum[i] - '0';
      if(strNum[i] == '/')
        iDigit = 10;

      iX += 18 * iScreenSizeX / 640;

      int iSourceX = iDigit * 18;
      int iDestX = iX;
      int iDestY = iY;
      drawtile(m_pStatNumbers->pData + iSourceX*4, // this should be very wrong, but it's not??
                m_pStatNumbers->iBytesPerLine,
                18,
                m_pStatNumbers->iHeight,
                0<<16, 0<<16,
                iDestX<<16, iDestY<<16, iTileScaleX, iTileScaleY, 0, -1);
    }
  }

  void Run(float fDeltaTime)
  {
    IntroScreen::Run(fDeltaTime);

    // has quit
    if(GetGameMenu()->m_ScreenStack.empty())
      return;

    if(m_CurrentState == State_Normal)
    {
      m_fLastCountTime += fDeltaTime;
      if(m_fLastCountTime > 0.1f)
      {
        m_fLastCountTime = 0;

        if(m_iCurrentCounter < 4)
        {
          if(m_Counters[m_iCurrentCounter].iValue < m_Counters[m_iCurrentCounter].iTarget)
          {          
            m_Counters[m_iCurrentCounter].iValue += Max(m_Counters[m_iCurrentCounter].iTarget / 10, (unsigned int)1);
            if(m_Counters[m_iCurrentCounter].iValue > m_Counters[m_iCurrentCounter].iTarget)
            {
              m_Counters[m_iCurrentCounter].iValue = m_Counters[m_iCurrentCounter].iTarget;
            }

            m_pCountSound->Play();
          }
          else
          {
            m_fLastCountTime -= 0.5f;
            m_iCurrentCounter++;
          }
        }
      }
    }

    DrawStatNumber(m_Counters[0].iValue, m_Counters[0].iMax, 108, 295);
    DrawStatNumber(m_Counters[1].iValue, m_Counters[1].iMax, 108+13, 295+46);
    DrawStatNumber(m_Counters[2].iValue, m_Counters[2].iMax, 108+83, 295+92);
    DrawStatNumber(m_Counters[3].iValue, m_Counters[3].iMax, 108+34, 295+138);
  }
};


void GameMenu::OpenFinishScreen(void)
{
  m_ScreenStack.push_back(new FinishScreen());
}