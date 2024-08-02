
#pragma once

#include <libretro.h>

#include "VideoBackends/Software/SWOGLWindow.h"
#include "VideoBackends/Software/SWGfx.h"
#include "VideoBackends/Software/SWTexture.h"

#include "DolphinLibretro/Options.h"
#include "DolphinLibretro/Video.h"

class RSWGfx : public SW::SWGfx
{
public:
  RSWGfx(std::unique_ptr<SWOGLWindow> window) : SW::SWGfx::SWGfx(std::move(window))
  {
    backbuffer_width  = EFB_WIDTH * Libretro::Options::efbScale;
    backbuffer_height = EFB_HEIGHT * Libretro::Options::efbScale;

    UpdateActiveConfig();
  }
  bool IsHeadless() const override
  {
    return false;
  }
  void BindBackbuffer(const ClearColor& clear_color) override
  {
    if (g_presenter->SurfaceResizedTestAndClear())
      g_presenter->SetBackbuffer(backbuffer_width, backbuffer_height);
  }
  void ShowImage(const AbstractTexture* source_texture, const MathUtil::Rectangle<int>& source_rc) override
  {
    const SW::SWTexture* m_texture = static_cast<const SW::SWTexture*>(source_texture);
    const unsigned texture_width = m_texture->GetWidth();

    ResizePixelBuffer(texture_width * m_texture->GetHeight());
    FillPixelBuffer(m_texture->GetData(0, 0));

    Libretro::Video::video_cb(
      pixel_buffer.data(),
      source_rc.GetWidth(),
      source_rc.GetHeight(),
      texture_width * 4
    );
  }
  SurfaceInfo GetSurfaceInfo() const override
  {
    return {
      std::max(backbuffer_width, 1u),
      std::max(backbuffer_height, 1u),
      1.0f,
      AbstractTextureFormat::RGBA8
    };
  }

private:
  void ClearPixelBuffer()
  {
    memset(pixel_buffer.data(), 0, pixel_buffer.size() * sizeof(pixel_buffer[0]));
  }
  void ResizePixelBuffer(unsigned new_size)
  {
    if (new_size != pixel_buffer_size)
    {
      pixel_buffer.resize(new_size);
      pixel_buffer_size = new_size;

      ClearPixelBuffer();
    }
  }
  void FillPixelBuffer(const u8* rgba8_data)
  {
    for (unsigned i = 0, j = 0; i < pixel_buffer_size; i++, j += 4)
    {
      pixel_buffer[i] =
        (0xFF000000) |
        (rgba8_data[j + 0] << 16) |
        (rgba8_data[j + 1] << 8) |
        (rgba8_data[j + 2]);
    }
  }

  unsigned backbuffer_width;
  unsigned backbuffer_height;

  std::vector<u32> pixel_buffer;
  unsigned pixel_buffer_size;
};
