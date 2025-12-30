
#pragma once

#include <libretro.h>

namespace Libretro
{
namespace Audio
{
void Init();
void Shutdown();
unsigned int GetSampleRate();
unsigned int GetSamplesPerFrame();
extern retro_audio_sample_batch_t batch_cb;
}
}
