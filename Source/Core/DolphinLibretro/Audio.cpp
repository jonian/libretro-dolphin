
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

void Init()
{
  auto& system = Core::System::GetInstance();
  g_sound_stream = system.GetSoundStream();
}

void Shutdown()
{
  g_sound_stream = nullptr;
  delete g_sound_stream;
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
