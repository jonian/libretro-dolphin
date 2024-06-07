
#pragma once

#include <libretro.h>

#include "AudioCommon/AudioCommon.h"

#include "DolphinLibretro/Options.h"
#include "DolphinLibretro/Audio.h"

class RSoundStream final : public SoundStream
{
public:
  RSoundStream() : SoundStream(Libretro::Audio::GetSampleRate()) {}
  bool Init() override
  {
    return true;
  }
  bool SetRunning(bool running) override
  {
    return running;
  }
  void Update() override
  {
    unsigned int available = m_mixer->AvailableSamples();
    while (available > MAX_SAMPLES)
    {
      m_mixer->Mix(m_buffer, MAX_SAMPLES);
      Libretro::Audio::batch_cb(m_buffer, MAX_SAMPLES);
      available -= MAX_SAMPLES;
    }
    if (available)
    {
      m_mixer->Mix(m_buffer, available);
      Libretro::Audio::batch_cb(m_buffer, available);
    }
  }

private:
  static constexpr unsigned int MAX_SAMPLES = 512;
  s16 m_buffer[MAX_SAMPLES * 2];
};
