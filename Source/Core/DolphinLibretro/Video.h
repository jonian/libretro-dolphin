
#pragma once

#include <libretro.h>

namespace Libretro
{
namespace Video
{
void Init(void);
extern retro_video_refresh_t video_cb;
extern struct retro_hw_render_callback hw_render;
}  // namespace Video
}  // namespace Libretro
