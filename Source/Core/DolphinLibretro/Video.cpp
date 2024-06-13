// needs to be first
#include "DolphinLibretro/Video.h"

#include <cassert>
#include <libretro.h>
#include <memory>
#include <sstream>
#include <vector>

#include "Common/GL/GLContext.h"
#include "Common/GL/GLUtil.h"
#include "Common/Logging/Log.h"
#include "Common/Version.h"
#include "Core/Config/GraphicsSettings.h"
#include "Core/Config/MainSettings.h"
#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/Host.h"
#include "DolphinLibretro/Options.h"
#include "VideoBackends/OGL/OGLGfx.h"
#include "VideoBackends/OGL/VideoBackend.h"
#include "VideoCommon/AsyncRequests.h"
#include "VideoCommon/Fifo.h"
#include "VideoCommon/RenderBase.h"
#include "VideoCommon/VertexLoaderManager.h"
#include "VideoCommon/VideoBackendBase.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

namespace Libretro
{
extern retro_environment_t environ_cb;
namespace Video
{
retro_video_refresh_t video_cb;
struct retro_hw_render_callback hw_render;

static void ContextReset(void)
{
  DEBUG_LOG_FMT(VIDEO, "Context reset!");

  WindowSystemInfo wsi(WindowSystemType::Libretro, nullptr, nullptr, nullptr);
  wsi.render_surface_scale = Libretro::Options::efbScale;

  g_video_backend->Initialize(wsi);
}

static void ContextDestroy(void)
{
  DEBUG_LOG_FMT(VIDEO, "Context destroy!");

  if (Config::Get(Config::MAIN_GFX_BACKEND) == "OGL")
  {
    OGL::GetOGLGfx()->SetSystemFrameBuffer(0);
  }

  g_video_backend->Shutdown();
}

static bool SetHWRender(retro_hw_context_type type)
{
  hw_render.context_type = type;
  hw_render.context_reset = ContextReset;
  hw_render.context_destroy = ContextDestroy;
  hw_render.bottom_left_origin = true;
  switch (type)
  {
  case RETRO_HW_CONTEXT_OPENGL_CORE:
    // minimum requirements to run is opengl 3.3 (RA will try to use highest version available anyway)
    hw_render.version_major = 3;
    hw_render.version_minor = 3;
    if (environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
    {
      Config::SetBase(Config::MAIN_GFX_BACKEND, "OGL");
      return true;
    }
    break;
  case RETRO_HW_CONTEXT_OPENGLES3:
  case RETRO_HW_CONTEXT_OPENGL:
    // when using RETRO_HW_CONTEXT_OPENGL you can't set version above 3.0 (RA will try to use highest version available anyway)
    // dolphin support OpenGL ES 3.0 too (no support for 2.0) so we are good
    hw_render.version_major = 3;
    hw_render.version_minor = 0;
    if (environ_cb(RETRO_ENVIRONMENT_SET_HW_RENDER, &hw_render))
    {
      // Shared context is required with "gl" video driver
      environ_cb(RETRO_ENVIRONMENT_SET_HW_SHARED_CONTEXT, nullptr);
      Config::SetBase(Config::MAIN_GFX_BACKEND, "OGL");
      return true;
    }
    break;
  default:
    break;
  }
  return false;
}
void Init()
{
  retro_hw_context_type preferred = RETRO_HW_CONTEXT_NONE;
  environ_cb(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, &preferred);

  if (preferred && SetHWRender(preferred))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGL_CORE))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGL))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGLES3))
    return;

  hw_render.context_type = RETRO_HW_CONTEXT_NONE;
}

void Shutdown()
{
  g_video_backend->Shutdown();

  g_video_backend = nullptr;
  delete g_video_backend;
}

}  // namespace Video
}  // namespace Libretro

void retro_set_video_refresh(retro_video_refresh_t cb)
{
  Libretro::Video::video_cb = cb;
}
