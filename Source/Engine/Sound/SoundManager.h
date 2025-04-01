#pragma once

class Sound;
class SoundData;

/// Manager for handling sounds
class SoundManager
{
public:
  SoundManager();
  ~SoundManager();

  /// Loads sound data from file
  SmartPointer<SoundData> GetSoundData(const std::string &strSound);
  /// Creates a streaming sound from file (a reference is kept until sound stops)
  inline SmartPointer<Sound> CreateStreamingSound(const std::string &strSound) { return CreateSound(strSound, false, true); }
  /// Creates a 2D sound from file (a reference is kept until sound stops)
  inline SmartPointer<Sound> CreateSound2D(const std::string &strSound) { return CreateSound(strSound, false, false); }
  /// Creates a 3D sound from file (a reference is kept until sound stops)
  inline SmartPointer<Sound> CreateSound(const std::string &strSound) { return CreateSound(strSound, true, false); }

  /// Updates manager
  void Update(void);

private:
  /// Creates a sound from file (a reference is kept until sound stops)
  SmartPointer<Sound> CreateSound(const std::string &strSound, bool bSound3D, bool bStreaming);

public:
  typedef std::map<std::string, SoundData *> SoundDataMap;
  /// Loaded sound data
  SoundDataMap m_SoundDataMap;

  /// Sounds still playing
  std::vector<Sound *> m_PlayingSounds;
  /// Free sounds that can be reused
  std::vector<Sound *> m_FreeSounds;
};

extern SoundManager *g_pSoundManager;
inline SoundManager *GetSoundManager(void) { return g_pSoundManager; }
