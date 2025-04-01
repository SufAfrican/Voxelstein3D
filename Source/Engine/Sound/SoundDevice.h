#pragma once

/// Sound device wrapper
class SoundDevice
{
public:
  SoundDevice();
  ~SoundDevice();

  /// Sound device settings
  struct DeviceSettings
  {
    DeviceSettings() : iChannels(2), iFrequency(22050), iBits(16) {}
    /// Number of channels (mono/stereo)
    int iChannels;
    /// Sampling rate
    int iFrequency;
    /// Bits per sample
    int iBits;
  };

  /// Initializes sound device
  bool Create(const DeviceSettings &settings);
  /// Releases sound device
  void Destroy(void);

  /// Sets listener parameters
  void SetListener(const Matrix44 &mTrans);
  /// Returns listener position
  inline const Vector3D &GetListener(void) { return m_vListener; }

  /// Updates device
  void Update(void);

  /// Returns settings device was created with
  inline const DeviceSettings &GetSettings(void) const { return m_Settings; }

  /// Returns API specific device
  void *GetDevice(void);

private:
  /// Settings device was created with
  DeviceSettings m_Settings;

  /// Listener position
  Vector3D m_vListener;

private:
  struct Data;
  /// API specific data
  Data *m_pData;
};

extern SoundDevice *g_pSoundDevice;
inline SoundDevice *GetSoundDevice(void) { return g_pSoundDevice; }
