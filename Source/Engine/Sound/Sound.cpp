#include "StdAfx.h"
#include "Engine/Sound/Sound.h"
#include "Engine/Sound/SoundData.h"
#include "Engine/Sound/SoundDevice.h"

#ifdef SOUND_OPENAL
  #include <al.h>
  #include <alc.h>

  struct Sound::Data
  {
    Data() { iSource = -1; }
    ALuint iSource;
  };

#else
  #include <windows.h>
  #include <mmsystem.h>
  #define DIRECTSOUND_VERSION 0x0900
  #include "dsound.h"

  struct Sound::Data
  {
    Data() { ZeroMemory(this, sizeof(Data)); }

    LPDIRECTSOUND3DBUFFER pBuffer3D;
    LPDIRECTSOUNDBUFFER pBuffer;
    int iNextStreamSlice;
    int iNextStreamPos;
  };

#endif


Sound::Sound()
{
  m_pData = new Data();
  m_pSoundData = NULL;
  m_iReferenceCount = 0;
  m_fVolume = 1.0f;
  m_fFadeOutDistanceSqr = FLT_MAX;
  m_bStreaming = false;
  m_bLooping = false;
  m_iMaxDataSize = 0;
}

Sound::~Sound()
{
  Destroy();
  delete m_pData;
}

bool Sound::Create(SoundData *pSoundData, bool bSound3D, unsigned int iMaxDataSize)
{
  m_pSoundData = pSoundData;
  m_bLooping = false;
  m_iMaxDataSize = iMaxDataSize;
  if(iMaxDataSize == -1)
  {
    m_bStreaming = false;
  }
  else
  {
    m_bStreaming = true;
    bSound3D = false;
    m_pData->iNextStreamPos = iMaxDataSize;
    m_pData->iNextStreamSlice = 0;
  }

  // no sound
  if(pSoundData->GetSampleData().empty())
    return false;

#ifdef SOUND_OPENAL

  // generate source
  alGenSources(1, &m_pData->iSource);
  if(m_pData->iSource == -1)
  {
    printf("Sound::Create - Failed to generate source for %s (error %x)\n", pSoundData->GetFilename().c_str(), alGetError());
    return false;
  }

  // set buffer
  alGetError();
  alSourcei(m_pData->iSource, AL_BUFFER, *(ALuint*)pSoundData->GetBuffer());
  ALenum iError = alGetError();
  if(iError != AL_NO_ERROR)
  {
    printf("Sound::Create - Setting buffer for %s failed with code %x\n", pSoundData->GetFilename().c_str(), iError);
    return false;
  }

  // set defaults
  alSourcef(m_pData->iSource, AL_REFERENCE_DISTANCE, 100.0f);

#else

  // setup format
  WAVEFORMATEX wfx;
  ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
  wfx.wFormatTag = WAVE_FORMAT_PCM;
  wfx.nChannels = pSoundData->GetChannels();
  wfx.nSamplesPerSec = pSoundData->GetFrequency();
  wfx.wBitsPerSample = 16;
  wfx.nBlockAlign = (WORD)(wfx.wBitsPerSample / 8 * wfx.nChannels);
  wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

  // setup buffer
  DSBUFFERDESC dsbd;
  ZeroMemory(&dsbd, sizeof(DSBUFFERDESC));
  dsbd.dwSize = sizeof(DSBUFFERDESC);
  dsbd.dwFlags = DSBCAPS_CTRLVOLUME | DSBCAPS_GLOBALFOCUS;
  if(m_bStreaming)
    dsbd.dwFlags |= DSBCAPS_GETCURRENTPOSITION2;
  if(bSound3D)
    dsbd.dwFlags |= DSBCAPS_CTRL3D;

  dsbd.dwBufferBytes = Min(pSoundData->GetSampleData().size(), iMaxDataSize);
  dsbd.lpwfxFormat = &wfx;

  // create buffer
  HRESULT hr;
  hr = ((LPDIRECTSOUND)GetSoundDevice()->GetDevice())->CreateSoundBuffer(&dsbd, &m_pData->pBuffer, NULL); 
  if(FAILED(hr))
  {
    printf("Sound::Create - CreateSoundBuffer failed with code %x\n", hr);
    return false;
  }

  // get 3D sound interface
  if(bSound3D)
  {
    hr = m_pData->pBuffer->QueryInterface(IID_IDirectSound3DBuffer, (LPVOID*)&m_pData->pBuffer3D);
    if(FAILED(hr))
    {
      printf("Sound::Create - QueryInterface failed with code %x\n", hr);
      return false;
    }
  }

  // lock buffer
  LPVOID pBufferData = NULL;
  DWORD iBufferLength = 0;
  hr = m_pData->pBuffer->Lock(0, 0, &pBufferData, &iBufferLength, NULL, NULL, DSBLOCK_ENTIREBUFFER);
  if(FAILED(hr))
  {
    printf("Sound::Create - Failed to lock buffer, error %x\n", hr);
    return false;
  }

  // upload data
  if(iBufferLength < Min(pSoundData->GetSampleData().size(), iMaxDataSize))
  {
    printf("Sound::Create - DirectSound returned a buffer that is too small!\n");
    ZeroMemory(pBufferData, iBufferLength);
  }
  else
  {
    memcpy(pBufferData, &pSoundData->GetSampleData()[0], iBufferLength);
  }

  // unlock
  hr = m_pData->pBuffer->Unlock(pBufferData, iBufferLength, NULL, 0);
  if(FAILED(hr))
  {
    printf("Sound::Create - Failed to unlock buffer, error %x\n", hr);
    return false;
  }

  // set defaults
  if(m_pData->pBuffer3D)
  {
    m_pData->pBuffer3D->SetMinDistance(100.0f, DS3D_DEFERRED);
    m_pData->pBuffer3D->SetMaxDistance(1000.0f, DS3D_DEFERRED);
  }
#endif

  m_fVolume = 1.0f;
  m_fFadeOutDistanceSqr = FLT_MAX;

  return true;
}

void Sound::Destroy(void)
{
#ifdef SOUND_OPENAL

  if(m_pData->iSource != -1)
  {
    alDeleteSources(1, &m_pData->iSource);
    m_pData->iSource = -1;
  }

#else

  if(m_pData->pBuffer3D != NULL)
  {
    m_pData->pBuffer3D->Release();
    m_pData->pBuffer3D = NULL;
  }

  if(m_pData->pBuffer != NULL)
  {
    m_pData->pBuffer->Release();
    m_pData->pBuffer = NULL;
  }

  m_pData->iNextStreamSlice = 0;
  m_pData->iNextStreamPos = 0;

#endif
  m_pSoundData = NULL;
}

void Sound::SetPosition(const Vector3D &vPos)
{
#ifdef SOUND_OPENAL
  alSourcefv(m_pData->iSource, AL_POSITION, vPos.v);
#else
  if(m_pData->pBuffer3D)
  {
    m_pData->pBuffer3D->SetPosition(vPos.x, vPos.y, vPos.z, DS3D_DEFERRED);
  }
#endif
  m_vPosition = vPos;
}

void Sound::SetRange(float fMin, float fMax)
{
#ifdef SOUND_OPENAL
  alSourcef(m_pData->iSource, AL_REFERENCE_DISTANCE, fMin);
#else
  if(m_pData->pBuffer3D)
  {
    m_pData->pBuffer3D->SetMinDistance(fMin, DS3D_DEFERRED);
  }
#endif
  m_fFadeOutDistanceSqr = fMax*fMax;
}

void Sound::SetPitch(float fPitch)
{
#ifdef SOUND_OPENAL
  alSourcef(m_pData->iSource, AL_PITCH, fPitch);
#else
  if(m_pData->pBuffer)
  {
    m_pData->pBuffer->SetFrequency(m_pSoundData->GetFrequency() * fPitch);
  }
#endif
}

void Sound::SetVolume(float fVolume)
{
#ifdef SOUND_OPENAL
  alSourcef(m_pData->iSource, AL_GAIN, fVolume);
#else
  if(m_pData->pBuffer)
  {
    // convert to decibels
    m_pData->pBuffer->SetVolume(fVolume > 0.0f ? 1000 * log10f(fVolume) : DSBVOLUME_MIN);
  }
#endif

  m_fVolume = fVolume;
}

void Sound::SetLooping(bool bLooping)
{
  m_bLooping = bLooping;
}

void Sound::Play(void)
{
  ApplyDistanceAttenuation();
#ifdef SOUND_OPENAL
  alSourcePlay(m_pData->iSource);
  // todo: looping
#else
  if(m_pData->pBuffer)
  {
    m_pData->pBuffer->SetCurrentPosition(0);
    m_pData->pBuffer->Play(0,0,m_bStreaming || m_bLooping ? DSBPLAY_LOOPING : 0);
  }
#endif
  ApplyDistanceAttenuation();
}

void Sound::Play(const Vector3D &vPos, float fPitch)
{
  SetPosition(vPos);
  SetPitch(fPitch);
  Play();
}

void Sound::Stop(void)
{
#ifdef SOUND_OPENAL
  alSourceStop(m_pData->iSource);
#else
  if(m_pData->pBuffer)
  {
    m_pData->pBuffer->Stop();
  }
#endif
}

bool Sound::IsPlaying(void)
{
#ifdef SOUND_OPENAL
  ALint iState = AL_STOPPED;
  alGetSourcei(m_pData->iSource, AL_SOURCE_STATE, &iState);
  return iState == AL_PLAYING;
#else
  if(m_pData->pBuffer)
  {
    DWORD iStatus = 0;
    m_pData->pBuffer->GetStatus(&iStatus);
    return iStatus & DSBSTATUS_PLAYING;
  }
  return false;
#endif
}

void Sound::ApplyDistanceAttenuation(void)
{
  if(m_fFadeOutDistanceSqr == FLT_MAX)
    return;

  float fDistSqr = (GetSoundDevice()->GetListener() - m_vPosition).SquaredLength();
  float fVolume = 0;
  if(fDistSqr < m_fFadeOutDistanceSqr)
    fVolume = m_fVolume * (1.0f - fDistSqr/m_fFadeOutDistanceSqr);

#ifdef SOUND_OPENAL
  alSourcef(m_pData->iSource, AL_GAIN, fVolume);
#else
  if(m_pData->pBuffer)
  {
    // convert to decibels
    m_pData->pBuffer->SetVolume(fVolume > 0.0f ? 1000 * log10f(fVolume) : DSBVOLUME_MIN);
  }
#endif
}

void Sound::UpdateStreaming(void)
{
  if(!m_bStreaming)
    return;

#ifdef SOUND_OPENAL
  TODO!
#else

  if(m_pData->pBuffer == NULL)
    return;

  HRESULT hr;
  DWORD iCurrentPlayCursor = 0, iCurrentWriteCursor = 0;
  hr = m_pData->pBuffer->GetCurrentPosition(&iCurrentPlayCursor, &iCurrentWriteCursor);
  if(FAILED(hr))
  {
    printf("Sound::UpdateStreaming - GetCurrentPosition failed, error %x\n", hr);
    return;
  }

  int iCopyOffset = -1;

  // copying to first half
  if(m_pData->iNextStreamSlice == 0)
  {
    // played halfway through
    if(iCurrentPlayCursor > m_iMaxDataSize/2)
    {
      iCopyOffset = 0;
      m_pData->iNextStreamSlice = 1;
    }
  }
  // copying to second half
  else if(m_pData->iNextStreamSlice == 1)
  {
    if(iCurrentPlayCursor < m_iMaxDataSize/2)
    {
      iCopyOffset = m_iMaxDataSize/2;
      m_pData->iNextStreamSlice = 0;
    }
  }

  // not ready to copy
  if(iCopyOffset < 0)
    return;

  // lock buffer
  LPVOID pBufferData = NULL;
  DWORD iBufferLength = 0;
  hr = m_pData->pBuffer->Lock(iCopyOffset, m_iMaxDataSize/2, &pBufferData, &iBufferLength, NULL, NULL, 0);
  if(FAILED(hr))
  {
    printf("Sound::UpdateStreaming - Failed to lock buffer, error %x\n", hr);
    return;
  }

  // upload data
  if(iBufferLength < m_iMaxDataSize/2)
  {
    printf("Sound::UpdateStreaming - DirectSound returned a buffer that is too small!\n");
    ZeroMemory(pBufferData, iBufferLength);
  }
  else
  {
    SoundData *pSoundData = m_pSoundData;

    // copy as much data as possible
    unsigned int iCopyBytes = Min(pSoundData->GetSampleData().size() - m_pData->iNextStreamPos, (unsigned int)iBufferLength);
    if(iCopyBytes > 0)
    {
      memcpy(pBufferData, &pSoundData->GetSampleData()[m_pData->iNextStreamPos], iCopyBytes);
      m_pData->iNextStreamPos += iCopyBytes;
    }

    // didn't have enough data
    if(iCopyBytes < iBufferLength)
    {
      if(m_bLooping)
      {
        // start again from the beginning
        memcpy(((char *)pBufferData) + iCopyBytes, &pSoundData->GetSampleData()[0], iBufferLength - iCopyBytes);
        m_pData->iNextStreamPos = iBufferLength - iCopyBytes;
      }
      else
      {
        // fill with zeroes
        ZeroMemory(((char *)pBufferData) + iCopyBytes, iBufferLength - iCopyBytes);
      }
    }
  }

  // unlock
  hr = m_pData->pBuffer->Unlock(pBufferData, iBufferLength, NULL, 0);
  if(FAILED(hr))
  {
    printf("Sound::UpdateStreaming - Failed to unlock buffer, error %x\n", hr);
    return;
  }

#endif
}
