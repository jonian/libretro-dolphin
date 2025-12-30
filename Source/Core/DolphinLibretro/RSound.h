
#pragma once

#include <libretro.h>

#include "AudioCommon/AudioCommon.h"

#include "DolphinLibretro/Options.h"
#include "DolphinLibretro/Audio.h"

class RSoundStream final : public SoundStream
{
public:
  bool Init() override
  {
    GetMixer()->SetSampleRate(Libretro::Audio::GetSampleRate());
    return true;
  }
  bool SetRunning(bool running) override
  {
    return true;
  }
  void Update(unsigned int num_samples) override
  {
    if (Libretro::Options::audioCallback == 0)
      MixAndPush(num_samples);
  }
  void Render(unsigned int num_samples)
  {
    if (Libretro::Options::audioCallback == 1)
      MixAndPush(num_samples);
  }
  void MixAndPush(unsigned int num_samples)
  {
    unsigned int available = num_samples;
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
  static constexpr unsigned int MAX_SAMPLES = 1024;
  s16 m_buffer[MAX_SAMPLES * 2];
};
