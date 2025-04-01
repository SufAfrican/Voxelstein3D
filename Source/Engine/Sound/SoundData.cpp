#include "StdAfx.h"
#include "Engine/Sound/Sound.h"
#include "Engine/Sound/SoundData.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>

#ifdef SOUND_OPENAL
  #include <al.h>
  #include <alc.h>

  struct SoundData::Data
  {
    Data()
    {
      iBuffer = -1;
    }

    ~Data()
    {
      if(iBuffer != -1)
        alDeleteBuffers(1, &iBuffer);
    }

    ALuint iBuffer;
  };

#else

  struct SoundData::Data
  {
    Data() {}
  };

#endif

SoundData::SoundData()
{
  m_pData = new Data();
  m_iFrequency = 0;
  m_iChannels = 0;
  m_iReferenceCount = 0;
}

SoundData::~SoundData()
{
  delete m_pData;
}

bool SoundData::LoadFromOGG(const char *strFile)
{
  FILE *pFile = fopen(strFile, "rb");
  if(pFile == NULL)
  {
    printf("SoundData::LoadFromFile - Failed to open file %s\n", strFile);
    return false;
  }

  OggVorbis_File OggFile;

  // open file
  int iResult;
  iResult = ov_open_callbacks(pFile, &OggFile, NULL, 0, OV_CALLBACKS_NOCLOSE);
  if(iResult < 0)
  {
    char *strError = "Unknown error.";
    switch(iResult)
    {
      case OV_EREAD: strError = "A read from media returned an error."; break;
      case OV_ENOTVORBIS: strError = "Bitstream does not contain any Vorbis data."; break;
      case OV_EVERSION: strError = "Vorbis version mismatch."; break;
      case OV_EBADHEADER: strError = "Invalid Vorbis bitstream header."; break;
      case OV_EFAULT: strError = "Internal logic fault; indicates a bug or heap/stack corruption."; break;
    };
    printf("SoundData::LoadFromFile - Reading OGG file %s failed: %s (code %i)\n", strFile, strError, iResult);
    fclose(pFile);
    return false;
  }

  // get info
  vorbis_info *pInfo = ov_info(&OggFile, -1);
  if(pInfo == NULL)
  {
    printf("SoundData::LoadFromFile - Retrieving OGG info for %s failed\n", strFile);
    ov_clear(&OggFile);
    fclose(pFile);
    return false;
  }
  m_iChannels = pInfo->channels;
  m_iFrequency = pInfo->rate;

  // read data
  static char pTempBuffer[4096];
  while(true)
  {
    int iCurrentSection = 0;
    int iNumBytes = ov_read(&OggFile, pTempBuffer, 4096, 0, 2, 1, &iCurrentSection);
    if(iNumBytes == 0)
    {
      // end of file
      break;
    }

    // error in stream.. just ignore
    if(iNumBytes < 0)
      continue;

    // insert to array
    unsigned int iDataOffset = m_SampleData.size();
    m_SampleData.resize(iDataOffset + iNumBytes);
    memcpy(&m_SampleData[iDataOffset], pTempBuffer, iNumBytes);
  }
  ov_clear(&OggFile);
  fclose(pFile);
  return true;
}

// file functions for reading through zip
extern long kzopen (const char *filnam);
extern long kzread (void *buffer, long leng);
extern long kzseek (long offset, long whence);
extern void kzclose ();

bool SoundData::LoadWolf3DMusic(unsigned int iStartWL1)
{
  if(!kzopen("AUDIOT.WL1"))
  {
    printf("SoundData::LoadFromFile - Failed to open AUDIOT.WL1, please install Wolfenstein 3D\n");
    return false;
  }
  kzseek(iStartWL1, SEEK_SET);

  unsigned short iSize = 0;
  kzread(&iSize, 2);
  
  extern void adlibinit(long dasamplerate, long danumspeakers, long dabytespersample);
  extern void adlib0 (long i, long v);
  extern void adlibgetsample (void *sndptr, long numbytes);

  // initialize adlib
  adlibinit(44100,2,2);
  unsigned int iDataPos = 0;
  int iMax = 0;

  // instruction data in IMF file
  struct Instruction
  {
    unsigned char iReg;
    unsigned char iData;
    unsigned short iDelay;
  };

  // loop through all data
  for(unsigned short i = 0; i < iSize; i += sizeof(Instruction))
  {
    // read instruction
    Instruction ins;
    kzread(&ins.iReg, 1);
    kzread(&ins.iData, 1);
    kzread(&ins.iDelay, 2);

    // send to adlib
    adlib0(ins.iReg, ins.iData);

    if(ins.iDelay != 0)
    {
      // render sample
      unsigned int iDataBytes = ins.iDelay * 44100/700*2*2; // 700Hz, 2 channels, 16 bits
      m_SampleData.resize(iDataPos + iDataBytes);
      short *pSampleData = (short *)&m_SampleData[iDataPos];
      adlibgetsample(pSampleData, iDataBytes);

      // get max
      for(unsigned int j = 0; j < iDataBytes/2; j++)
      {
        int iSample = pSampleData[j];
        if(iSample < 0)
          iSample = -iSample;
        if(iSample > iMax)
          iMax = iSample;
      }
      iDataPos += iDataBytes;
    }
  }

  kzclose();

  // maximize
  short *pSampleData = (short *)&m_SampleData[0];
  for(unsigned int i = 0; i < m_SampleData.size()/2; i++)
  {
    pSampleData[i] = pSampleData[i] * SHRT_MAX / iMax;
  }

  m_iChannels = 2;
  m_iFrequency = 44100;
  return true;
}

/// Loads data from Wolfenstein3D
bool SoundData::LoadWolf3DSound(unsigned int iStartWL1, unsigned int iSizeWL1, unsigned int iStartWL6, unsigned int iSizeWL6)
{
  unsigned int iStart = 0;
  unsigned int iSize = 0;

  // convert from shareware to fullversion
  if(iStartWL1 != 0 && iStartWL6 == 0)
  {
    iStartWL6 = iStartWL1 + 0x86E00;
  }

  // load from full version
  bool bOpenedFull = kzopen("VSWAP.WL6") != 0;

  // got it
  if(bOpenedFull)
  {
    iStart = iStartWL6;
    iSize = iSizeWL6;
  }
  // full version required
  if(!bOpenedFull && iStartWL1 == 0)
  {
    printf("SoundData::LoadFromFile - Failed to open VSWAP.WL6, please install Wolfenstein 3D\n");
    return false;
  }
  // load from shareware version
  else if(!bOpenedFull)
  {
    if(!kzopen("VSWAP.WL1"))
    {
      printf("SoundData::LoadFromFile - Failed to open VSWAP.WL1, please install Wolfenstein 3D\n");
      return false;
    }

    iStart = iStartWL1;
    iSize = iSizeWL1;
  }
  
  m_iFrequency = 7000;
  m_iChannels = 1;

  if(iSize == 0)
  {
    kzclose();
    return true;
  }

  // read 8-bit sample data
  std::vector<unsigned char> SampleData8(iSize);
  kzseek(iStart, SEEK_SET);
  kzread(&SampleData8[0], iSize);

  // expand to 16 bit signed data
  m_SampleData.resize(iSize*2);
  short *pSampleData16 = (short *)&m_SampleData[0];
  for(unsigned int i = 0; i < iSize; i++)
  {
    int iSample16 = (SampleData8[i] ^ 0x80) << 8;
    pSampleData16[i] = iSample16;
  }

  kzclose();
  return true;
}

bool SoundData::LoadFromFile(const char *strFile)
{
  // wolfenstein sound parsing
  unsigned int iStartWL1, iSizeWL1, iStartWL6, iSizeWL6; // byte offsets
  if(sscanf(strFile,"Wolf3D:%u,%u,%u,%u", &iStartWL1, &iSizeWL1, &iStartWL6, &iSizeWL6) == 4)
  {
    if(!LoadWolf3DSound(iStartWL1, iSizeWL1, iStartWL6, iSizeWL6))
      return false;
  }
  else if(sscanf(strFile,"Wolf3D:mus%u", &iStartWL1) == 1)
  {
    if(!LoadWolf3DMusic(iStartWL1))
      return false;
  }
  else
  {
    // ogg sound
    if(!LoadFromOGG(strFile))
      return false;
  }

#ifdef SOUND_OPENAL
  // create buffer
  m_pData->iBuffer = -1;
  alGenBuffers(1, &m_pData->iBuffer);
  if(m_pData->iBuffer == -1)
  {
    printf("SoundData::LoadFromFile - Generating buffer for %s failed with code %x\n", strFile, alGetError());
    return false;
  }

  // buffer data
  alGetError();
  alBufferData(m_pData->iBuffer, m_iChannels == 2 ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16,
                                &m_SampleData[0], m_SampleData.size(),
                                m_iFrequency);
  ALenum iError = alGetError();
  if(iError != AL_NO_ERROR)
  {
    printf("SoundData::LoadFromFile - Buffering data for %s failed with code %x\n", strFile, iError);
    return false;
  }
#endif

  m_strFilename = strFile;
  return true;
}

void *SoundData::GetBuffer(void)
{
#ifdef SOUND_OPENAL
  return &m_pData->iBuffer;
#else
  return NULL;
#endif
}