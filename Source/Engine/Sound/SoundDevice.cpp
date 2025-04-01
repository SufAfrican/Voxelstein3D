#include "StdAfx.h"
#include "Engine/Sound/SoundDevice.h"

#ifdef SOUND_OPENAL
  #include <al.h>
  #include <alc.h>

  struct SoundDevice::Data
  {
    Data() { memset(this, 0, sizeof(Data)); }
    ALCdevice *pDevice;
    ALCcontext *pContext;
  };

#pragma comment(lib,"OpenAL32.lib")

#else
  #include <windows.h>
  #include <mmsystem.h>
  #define DIRECTSOUND_VERSION 0x0900
  #include "dsound.h"
  extern HWND ghwnd;

  struct SoundDevice::Data
  {
    Data() { ZeroMemory(this, sizeof(Data)); }
    LPDIRECTSOUND3DLISTENER pListener;
    LPDIRECTSOUNDBUFFER pPrimaryBuffer;
    LPDIRECTSOUND pDirectSound;
  };

#endif

SoundDevice *g_pSoundDevice = NULL;

SoundDevice::SoundDevice()
{
  m_pData = new Data();
  m_vListener = Vector3D(0,0,0);
}

SoundDevice::~SoundDevice()
{
  Destroy();
  delete m_pData;
}

bool SoundDevice::Create(const DeviceSettings &settings)
{
  m_Settings = settings;
#ifdef SOUND_OPENAL
  // open device
  m_pData->pDevice = alcOpenDevice(NULL);
  if(m_pData->pDevice == NULL)
  {
    printf("SoundDevice::Create - alcOpenDevice failed with code %x\n", alGetError());
    return false;
  }

  // create context
  m_pData->pContext = alcCreateContext(m_pData->pDevice, NULL); 
  if(m_pData->pContext == NULL)
  {
    printf("SoundDevice::Create - alcCreateContext failed with code %x\n", alGetError());
    return false;
  }

  // activate context
  if(!alcMakeContextCurrent(m_pData->pContext))
  {
    printf("SoundDevice::Create - alcMakeContextCurrent failed with code %x\n", alGetError());
    return false;
  }
  
  // set stuff
  alSpeedOfSound(0.03f);

#else

  HRESULT hr;

  // create device
  hr = DirectSoundCreate(NULL,&m_pData->pDirectSound,NULL);
  if(FAILED(hr))
  {
    printf("SoundDevice::Create - DirectSoundCreate failed with code %x\n", hr);
    return false;
  }

  // set cooperative level
  hr = m_pData->pDirectSound->SetCooperativeLevel(ghwnd, DSSCL_PRIORITY);
  if(FAILED(hr))
  {
    printf("SoundDevice::Create - SetCooperativeLevel failed with code %x\n", hr);
    return false;
  }

  // create primary buffer
  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRL3D|DSBCAPS_PRIMARYBUFFER;
  dsbd.dwBufferBytes = 0;
  dsbd.lpwfxFormat = NULL;
  hr = m_pData->pDirectSound->CreateSoundBuffer(&dsbd, &m_pData->pPrimaryBuffer, NULL);
  if(FAILED(hr))
  {
    printf("SoundDevice::Create - CreateSoundBuffer for primary buffer failed with code %x\n", hr);
    return false;
  }

  // set format
  WAVEFORMATEX wfx;
  ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
  wfx.wFormatTag = (WORD)WAVE_FORMAT_PCM;
  wfx.nChannels = (WORD)settings.iChannels;
  wfx.nSamplesPerSec = (DWORD)settings.iFrequency;
  wfx.wBitsPerSample = (WORD)settings.iBits;
  wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
  wfx.nAvgBytesPerSec = (DWORD)(wfx.nSamplesPerSec * wfx.nBlockAlign);
  hr = m_pData->pPrimaryBuffer->SetFormat(&wfx);
  if(FAILED(hr))
  {
    printf("SoundDevice::Create - SetFormat for primary buffer failed with code %x\n", hr);
    return false;
  }

  // get listener
  hr = m_pData->pPrimaryBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID*)&m_pData->pListener);
  if(FAILED(hr))
  {
    printf("SoundDevice::Create - QueryInterface for listener failed with code %x\n", hr);
    return false;
  }

  m_pData->pListener->SetDistanceFactor(0.05f, DS3D_IMMEDIATE);

#endif

  return true;
}

void SoundDevice::Destroy(void)
{
#ifdef SOUND_OPENAL

  if(m_pData->pContext)
  {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(m_pData->pContext);
    m_pData->pContext = NULL;
  }
  if(m_pData->pDevice)
  {
    alcCloseDevice(m_pData->pDevice);
    m_pData->pDevice = NULL;
  }

#else

  if(m_pData->pListener)
  {
    m_pData->pListener->Release();
    m_pData->pListener = NULL;
  }
  if(m_pData->pPrimaryBuffer)
  {
    m_pData->pPrimaryBuffer->Release();
    m_pData->pPrimaryBuffer = NULL;
  }
  if(m_pData->pDirectSound)
  {
    m_pData->pDirectSound->Release();
    m_pData->pDirectSound = NULL;
  }

#endif
}

void SoundDevice::SetListener(const Matrix44 &mTrans)
{
  Vector3D vPos = mTrans.GetTranslation();
  Vector3D vFront = mTrans.GetFrontVector();
  Vector3D vUp = mTrans.GetUpVector();

  m_vListener = vPos;

#ifdef SOUND_OPENAL
  ALfloat pPosition[] = { vPos.x, vPos.y, vPos.z };
  ALfloat pOrientation[] = { -vFront.x, -vFront.y, -vFront.z, vUp.x, vUp.y, vUp.z };
  alListenerfv(AL_POSITION, pPosition);
  alListenerfv(AL_ORIENTATION, pOrientation);

#else
  m_pData->pListener->SetPosition(vPos.x, vPos.y, vPos.z, DS3D_DEFERRED);
  m_pData->pListener->SetOrientation(vFront.x, vFront.y, vFront.z, vUp.x, vUp.y, vUp.z, DS3D_DEFERRED);
#endif
}

void *SoundDevice::GetDevice(void)
{
#ifdef SOUND_OPENAL
  return m_pData->pDevice;
#else
  return m_pData->pDirectSound;
#endif
}

void SoundDevice::Update(void)
{
#ifdef SOUND_OPENAL

#else
  m_pData->pListener->CommitDeferredSettings();
#endif
}