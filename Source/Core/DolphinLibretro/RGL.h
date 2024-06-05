
#pragma once

#include <libretro.h>

#include "Common/GL/GLContext.h"

#include "DolphinLibretro/Options.h"
#include "DolphinLibretro/Video.h"

class GLContextRGL : public GLContext
{
public:
  GLContextRGL()
  {
    WindowSystemInfo wsi(WindowSystemType::Libretro, nullptr, nullptr, nullptr);
    Initialize(wsi, g_Config.stereo_mode == StereoMode::QuadBuffer, true);
  }
  bool IsHeadless() const override
  {
    return false;
  }
  void Swap() override
  {
    Libretro::Video::video_cb(RETRO_HW_FRAME_BUFFER_VALID, m_backbuffer_width, m_backbuffer_height, 0);
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
