
#include <cstdint>
#include <string>

#include "AudioCommon/AudioCommon.h"
#include "Core/ConfigManager.h"
#include "Core/System.h"
#include "DolphinLibretro/Options.h"
#include "DolphinLibretro/Audio.h"

SoundStream* g_sound_stream;

namespace Libretro
{
namespace Audio
{
retro_audio_sample_batch_t batch_cb;

unsigned int GetSampleRate()
{
  if (g_sound_stream)
    return g_sound_stream->GetMixer()->GetSampleRate();
  else if (Core::System::GetInstance().IsWii())
    return Libretro::Options::audioMixerRate;
  else if (Libretro::Options::audioMixerRate == 32000u)
    return 32029;

  return 48043;
}

class Stream final : public SoundStream
{
public:
  Stream() : SoundStream(GetSampleRate()) {}
  bool SetRunning(bool running) override { return running; }
  void Update() override
  {
    unsigned int available = m_mixer->AvailableSamples();
    while (available > MAX_SAMPLES)
    {
      m_mixer->Mix(m_buffer, MAX_SAMPLES);
      batch_cb(m_buffer, MAX_SAMPLES);
      available -= MAX_SAMPLES;
    }
    if (available)
    {
      m_mixer->Mix(m_buffer, available);
      batch_cb(m_buffer, available);
    }
  }

private:
  static constexpr unsigned int MAX_SAMPLES = 512;
  s16 m_buffer[MAX_SAMPLES * 2];
};

void Init()
{
  auto& system = Core::System::GetInstance();
  AudioCommon::SetSoundStreamRunning(system, false);
  system.SetSoundStream(std::make_unique<Libretro::Audio::Stream>());
  g_sound_stream = system.GetSoundStream();
  AudioCommon::SetSoundStreamRunning(system, true);
}

void Shutdown()
{
  g_sound_stream = nullptr;
}

}  // namespace Audio
}  // namespace Libretro

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
  Libretro::Audio::batch_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
}
