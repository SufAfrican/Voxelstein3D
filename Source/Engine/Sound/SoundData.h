#pragma once

/// Holds the shared data of a sound sample
class SoundData
{
public:
  SoundData();
  ~SoundData();

  /// Loads sample data from file
  bool LoadFromFile(const char *strFile);

  /// Returns number of channels
  inline int GetChannels(void) { return m_iChannels; }
  /// Returns sampling rate
  inline int GetFrequency(void) { return m_iFrequency; }
  /// Returns sample data
  inline const std::vector<unsigned char> &GetSampleData(void) { return m_SampleData; }
  /// Returns filename
  inline const std::string &GetFilename(void) { return m_strFilename; }

  /// Returns API specific buffer pointer
  void *GetBuffer(void);

public:
  /// Increases reference count
  inline void AddReference(void) { m_iReferenceCount++; }
  /// Decreases reference count
  inline void RemoveReference(void) { m_iReferenceCount--; if(m_iReferenceCount == 0) delete this; }
  /// Reference count
  int m_iReferenceCount;

private:
  /// Loads data from OGG file
  bool LoadFromOGG(const char *strFile);
  /// Loads Adlib sound data from Wolfenstein 3D
  bool LoadWolf3DMusic(unsigned int iStartWL1);
  /// Loads PCM sound data from Wolfenstein 3D
  bool LoadWolf3DSound(unsigned int iStartWL1, unsigned int iSizeWL1, unsigned int iStartWL6, unsigned int iSizeWL6);

private:
  /// Number of channels
  int m_iChannels;
  /// Sampling rate
  int m_iFrequency;
  /// Sample data
  std::vector<unsigned char> m_SampleData;
  /// Filename
  std::string m_strFilename;

  struct Data;
  /// API specific data
  Data *m_pData;
};