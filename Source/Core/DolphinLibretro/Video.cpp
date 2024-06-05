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
#include "VideoBackends/OGL/OGLRender.h"
#include "VideoBackends/OGL/VideoBackend.h"
#include "VideoCommon/AsyncRequests.h"
#include "VideoCommon/Fifo.h"
#include "VideoCommon/RenderBase.h"
#include "VideoCommon/VertexLoaderManager.h"
#include "VideoCommon/VideoBackendBase.h"
#include "VideoCommon/VideoCommon.h"
#include "VideoCommon/VideoConfig.h"

/* retroGL interface*/

class RGLContext : public GLContext
{
public:
  RGLContext()
  {
    WindowSystemInfo wsi(WindowSystemType::Libretro, nullptr, nullptr, nullptr);
    Initialize(wsi, g_Config.stereo_mode == StereoMode::QuadBuffer, true);
  }
  bool IsHeadless() const override { return false; }
  void Swap() override
  {
    Libretro::Video::video_cb(RETRO_HW_FRAME_BUFFER_VALID, m_backbuffer_width, m_backbuffer_height,
                              0);
  }
  void* GetFuncAddress(const std::string& name) override
  {
    return (void*)Libretro::Video::hw_render.get_proc_address(name.c_str());
  }
  virtual bool Initialize(const WindowSystemInfo& wsi, bool stereo, bool core) override
  {
    m_backbuffer_width = EFB_WIDTH * Libretro::Options::efbScale;
    m_backbuffer_height = EFB_HEIGHT * Libretro::Options::efbScale;
    switch (Libretro::Video::hw_render.context_type)
    {
    case RETRO_HW_CONTEXT_OPENGLES3:
      m_opengl_mode = Mode::OpenGLES;
      break;
    default:
    case RETRO_HW_CONTEXT_OPENGL_CORE:
    case RETRO_HW_CONTEXT_OPENGL:
      m_opengl_mode = Mode::OpenGL;
      break;
    }

    return true;
  }
};

namespace Libretro
{
extern retro_environment_t environ_cb;
namespace Video
{
retro_video_refresh_t video_cb;
struct retro_hw_render_callback hw_render;

static void ContextReset(void)
{
  DEBUG_LOG_FMT(VIDEO, "Context reset!\n");

  if (Config::Get(Config::MAIN_GFX_BACKEND) == "OGL")
  {
    std::unique_ptr<GLContext> main_gl_context = std::make_unique<RGLContext>();

    OGL::VideoBackend* ogl = static_cast<OGL::VideoBackend*>(g_video_backend);
    ogl->InitializeGLExtensions(main_gl_context.get());
    ogl->FillBackendInfo();
    ogl->InitializeShared();
    g_renderer =
        std::make_unique<OGL::Renderer>(std::move(main_gl_context), Libretro::Options::efbScale);
  }

  WindowSystemInfo wsi(WindowSystemType::Libretro, nullptr, nullptr, nullptr);
  g_video_backend->Initialize(wsi);
}

static void ContextDestroy(void)
{
  DEBUG_LOG_FMT(VIDEO, "Context destroy!\n");

  if (Config::Get(Config::MAIN_GFX_BACKEND) == "OGL")
  {
    static_cast<OGL::Renderer*>(g_renderer.get())->SetSystemFrameBuffer(0);
  }

  g_video_backend->Shutdown();
  switch (hw_render.context_type)
  {
  case RETRO_HW_CONTEXT_DIRECT3D:
    break;
  case RETRO_HW_CONTEXT_VULKAN:
    break;
  case RETRO_HW_CONTEXT_OPENGL:
  case RETRO_HW_CONTEXT_OPENGL_CORE:
  case RETRO_HW_CONTEXT_OPENGLES3:
    break;
  default:
  case RETRO_HW_CONTEXT_NONE:
    break;
  }
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

  if (environ_cb(RETRO_ENVIRONMENT_GET_PREFERRED_HW_RENDER, &preferred) && SetHWRender(preferred))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGL_CORE))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGL))
    return;
  if (SetHWRender(RETRO_HW_CONTEXT_OPENGLES3))
    return;

  hw_render.context_type = RETRO_HW_CONTEXT_NONE;
}

}  // namespace Video
}  // namespace Libretro

void retro_set_video_refresh(retro_video_refresh_t cb)
{
  Libretro::Video::video_cb = cb;
}
