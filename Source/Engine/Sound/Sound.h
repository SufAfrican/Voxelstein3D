#pragma once

class SoundData;

/// An unique instance of a sound
class Sound
{
public:
  Sound();
  ~Sound();

  /// Creates sound from data, use iMaxDataSize to create a streaming sound
  bool Create(SoundData *pSoundData, bool bSound3D = true, unsigned int iMaxDataSize = -1);
  /// Destroys sound
  void Destroy(void);

  /// Sets sound position
  void SetPosition(const Vector3D &vPos);
  /// Sets sound range
  void SetRange(float fMin, float fMax);

  /// Sets sound pitch
  void SetPitch(float fPitch);
  /// Sets sound volume
  void SetVolume(float fVolume);
  /// Enables/disables looping
  void SetLooping(bool bLooping);

  /// Plays sound
  void Play(void);
  /// Plays sound, sets position and pitch
  void Play(const Vector3D &vPos, float fPitch);
  /// Stops sound
  void Stop(void);

  /// Returns true if sound is playing
  bool IsPlaying(void);

  /// Updates sound buffer if the sound is streaming
  void UpdateStreaming(void);
  /// Applies additional distance attenuation
  void ApplyDistanceAttenuation(void);

public:
  /// Increases reference count
  inline void AddReference(void) { m_iReferenceCount++; }
  /// Decreases reference count
  inline void RemoveReference(void) { m_iReferenceCount--; if(m_iReferenceCount == 0) delete this; }
  /// Reference count
  int m_iReferenceCount;

private:
  struct Data;
  /// API specific data
  Data *m_pData;
  /// Sound data smart pointer
  SmartPointer<SoundData> m_pSoundData;
  /// Position where sound was played from
  Vector3D m_vPosition;
  /// Distance for manual attenuation
  float m_fFadeOutDistanceSqr;
  /// User set volume
  float m_fVolume;
  /// Sound is streaming
  bool m_bStreaming;
  /// Is sound looping
  bool m_bLooping;
  /// How many bytes of data in streaming
  unsigned int m_iMaxDataSize;
};
