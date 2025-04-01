#include "StdAfx.h"
#include "Engine/Sound/SoundDevice.h"
#include "Engine/Sound/SoundManager.h"
#include "Engine/Sound/SoundData.h"
#include "Engine/Sound/Sound.h"

SoundManager *g_pSoundManager = NULL;

SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
  // clear all references
  for(unsigned int i = 0; i < m_PlayingSounds.size(); i++)
  {
    m_PlayingSounds[i]->RemoveReference();
  }
  for(unsigned int i = 0; i < m_FreeSounds.size(); i++)
  {
    m_FreeSounds[i]->RemoveReference();
  }

  SoundDataMap::iterator it;
  for(it = m_SoundDataMap.begin(); it != m_SoundDataMap.end(); it++)
  {
    if(it->second->m_iReferenceCount > 1)
      printf("Warning: sound %s was not released!\n",it->first.c_str());
    it->second->RemoveReference();
  }
}

SmartPointer<SoundData> SoundManager::GetSoundData(const std::string &strSound)
{
  // find in map
  SoundDataMap::iterator it = m_SoundDataMap.find(strSound);
  if(it == m_SoundDataMap.end())
  {
    // create new
    SoundData *pSoundData = new SoundData();
    pSoundData->LoadFromFile(strSound.c_str());

    // add one reference for the manager
    pSoundData->AddReference();

    // add to map
    m_SoundDataMap[strSound] = pSoundData;
    return pSoundData;
  }

  // return existing
  return it->second;
}

SmartPointer<Sound> SoundManager::CreateSound(const std::string &strSound, bool bSound3D, bool bStreaming)
{
  Sound *pSound;

  // create new sound
  if(m_FreeSounds.empty())
  {
    pSound = new Sound();
    pSound->AddReference();
  }
  else
  {
    // reuse existing sound
    pSound = m_FreeSounds.back();
    m_FreeSounds.pop_back();
  }

  // load data
  bool bSuccess;
  if(bStreaming)
  {
    SoundData *pData = GetSoundData(strSound);
    bSuccess = pSound->Create(pData, false, pData ? pData->GetFrequency() * 2 : -1);
  }
  else
  {
    bSuccess = pSound->Create(GetSoundData(strSound), bSound3D);
  }

  if(!bSuccess)
    pSound->Destroy();

  // add to list
  m_PlayingSounds.push_back(pSound);
  return pSound;
}

void SoundManager::Update(void)
{
  for(unsigned int i = 0; i < m_PlayingSounds.size();)
  {
    Sound *pSound = m_PlayingSounds[i];

    // no more references and no longer playing
    if(pSound->m_iReferenceCount == 1 && !pSound->IsPlaying())
    {
      // insert into free sounds
      if(m_FreeSounds.size() < 10)
      {
        pSound->Destroy();
        m_FreeSounds.push_back(pSound);
      }
      // or just destroy
      else
      {
        pSound->RemoveReference();
      }

      m_PlayingSounds[i] = m_PlayingSounds.back();
      m_PlayingSounds.pop_back();
    }
    else
    {
      // update
      pSound->UpdateStreaming();
      pSound->ApplyDistanceAttenuation();
      i++;
    }
  }
}