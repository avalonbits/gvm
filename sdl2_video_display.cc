#include "sdl2_video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace {

// XTerm 256 color palette.
static const uint32_t kColorTable[256] = {
	0xFF000000, 0xFF000080, 0xFF008000, 0xFF008080, 0xFF800000, 0xFF800080, 0xFF808000, 0xFFC0C0C0, 
	0xFF808080, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF, 
	0xFF000000, 0xFF5F0000, 0xFF870000, 0xFFAF0000, 0xFFD70000, 0xFFFF0000, 0xFF005F00, 0xFF5F5F00, 
	0xFF875F00, 0xFFAF5F00, 0xFFD75F00, 0xFFFF5F00, 0xFF008700, 0xFF5F8700, 0xFF878700, 0xFFAF8700, 
	0xFFD78700, 0xFFFF8700, 0xFF00AF00, 0xFF5FAF00, 0xFF87AF00, 0xFFAFAF00, 0xFFD7AF00, 0xFFFFAF00, 
	0xFF00D700, 0xFF5FD700, 0xFF87D700, 0xFFAFD700, 0xFFD7D700, 0xFFFFD700, 0xFF00FF00, 0xFF5FFF00, 
	0xFF87FF00, 0xFFAFFF00, 0xFFD7FF00, 0xFFFFFF00, 0xFF00005F, 0xFF5F005F, 0xFF87005F, 0xFFAF005F, 
	0xFFD7005F, 0xFFFF005F, 0xFF005F5F, 0xFF5F5F5F, 0xFF875F5F, 0xFFAF5F5F, 0xFFD75F5F, 0xFFFF5F5F, 
	0xFF00875F, 0xFF5F875F, 0xFF87875F, 0xFFAF875F, 0xFFD7875F, 0xFFFF875F, 0xFF00AF5F, 0xFF5FAF5F, 
	0xFF87AF5F, 0xFFAFAF5F, 0xFFD7AF5F, 0xFFFFAF5F, 0xFF00D75F, 0xFF5FD75F, 0xFF87D75F, 0xFFAFD75F, 
	0xFFD7D75F, 0xFFFFD75F, 0xFF00FF5F, 0xFF5FFF5F, 0xFF87FF5F, 0xFFAFFF5F, 0xFFD7FF5F, 0xFFFFFF5F, 
	0xFF000087, 0xFF5F0087, 0xFF870087, 0xFFAF0087, 0xFFD70087, 0xFFFF0087, 0xFF005F87, 0xFF5F5F87, 
	0xFF875F87, 0xFFAF5F87, 0xFFD75F87, 0xFFFF5F87, 0xFF008787, 0xFF5F8787, 0xFF878787, 0xFFAF8787, 
	0xFFD78787, 0xFFFF8787, 0xFF00AF87, 0xFF5FAF87, 0xFF87AF87, 0xFFAFAF87, 0xFFD7AF87, 0xFFFFAF87, 
	0xFF00D787, 0xFF5FD787, 0xFF87D787, 0xFFAFD787, 0xFFD7D787, 0xFFFFD787, 0xFF00FF87, 0xFF5FFF87, 
	0xFF87FF87, 0xFFAFFF87, 0xFFD7FF87, 0xFFFFFF87, 0xFF0000AF, 0xFF5F00AF, 0xFF8700AF, 0xFFAF00AF, 
	0xFFD700AF, 0xFFFF00AF, 0xFF005FAF, 0xFF5F5FAF, 0xFF875FAF, 0xFFAF5FAF, 0xFFD75FAF, 0xFFFF5FAF, 
	0xFF0087AF, 0xFF5F87AF, 0xFF8787AF, 0xFFAF87AF, 0xFFD787AF, 0xFFFF87AF, 0xFF00AFAF, 0xFF5FAFAF, 
	0xFF87AFAF, 0xFFAFAFAF, 0xFFD7AFAF, 0xFFFFAFAF, 0xFF00D7AF, 0xFF5FD7AF, 0xFF87D7AF, 0xFFAFD7AF, 
	0xFFD7D7AF, 0xFFFFD7AF, 0xFF00FFAF, 0xFF5FFFAF, 0xFF87FFAF, 0xFFAFFFAF, 0xFFD7FFAF, 0xFFFFFFAF, 
	0xFF0000D7, 0xFF5F00D7, 0xFF8700D7, 0xFFAF00D7, 0xFFD700D7, 0xFFFF00D7, 0xFF005FD7, 0xFF5F5FD7, 
	0xFF875FD7, 0xFFAF5FD7, 0xFFD75FD7, 0xFFFF5FD7, 0xFF0087D7, 0xFF5F87D7, 0xFF8787D7, 0xFFAF87D7, 
	0xFFD787D7, 0xFFFF87D7, 0xFF00AFD7, 0xFF5FAFD7, 0xFF87AFD7, 0xFFAFAFD7, 0xFFD7AFD7, 0xFFFFAFD7, 
	0xFF00D7D7, 0xFF5FD7D7, 0xFF87D7D7, 0xFFAFD7D7, 0xFFD7D7D7, 0xFFFFD7D7, 0xFF00FFD7, 0xFF5FFFD7, 
	0xFF87FFD7, 0xFFAFFFD7, 0xFFD7FFD7, 0xFFFFFFD7, 0xFF0000FF, 0xFF5F00FF, 0xFF8700FF, 0xFFAF00FF, 
	0xFFD700FF, 0xFFFF00FF, 0xFF005FFF, 0xFF5F5FFF, 0xFF875FFF, 0xFFAF5FFF, 0xFFD75FFF, 0xFFFF5FFF, 
	0xFF0087FF, 0xFF5F87FF, 0xFF8787FF, 0xFFAF87FF, 0xFFD787FF, 0xFFFF87FF, 0xFF00AFFF, 0xFF5FAFFF, 
	0xFF87AFFF, 0xFFAFAFFF, 0xFFD7AFFF, 0xFFFFAFFF, 0xFF00D7FF, 0xFF5FD7FF, 0xFF87D7FF, 0xFFAFD7FF, 
	0xFFD7D7FF, 0xFFFFD7FF, 0xFF00FFFF, 0xFF5FFFFF, 0xFF87FFFF, 0xFFAFFFFF, 0xFFD7FFFF, 0xFFFFFFFF, 
	0xFF080808, 0xFF121212, 0xFF1C1C1C, 0xFF262626, 0xFF303030, 0xFF3A3A3A, 0xFF444444, 0xFF4E4E4E, 
	0xFF585858, 0xFF626262, 0xFF6C6C6C, 0xFF767676, 0xFF808080, 0xFF8A8A8A, 0xFF949494, 0xFF9E9E9E, 
	0xFFA8A8A8, 0xFFB2B2B2, 0xFFBCBCBC, 0xFFC6C6C6, 0xFFD0D0D0, 0xFFDADADA, 0xFFE4E4E4, 0xFFEEEEEE
};
}  // namespace

namespace gvm {

SDL2VideoDisplay::SDL2VideoDisplay() : SDL2VideoDisplay(800, 600, true, "") {}

SDL2VideoDisplay::SDL2VideoDisplay(int width, int height)
  : SDL2VideoDisplay(width, height, false, "") {
    count_ = 0;
}

SDL2VideoDisplay::SDL2VideoDisplay(
    int width, int height, const bool fullscreen, const std::string force_driver)
  : texture_(nullptr) {

  // The text buffer is 96x27 chars wide, with each char uisng 4 bytes (2 for
  // char, 1 for fgcolor, 1 for bg color.
  text_vram_buffer_ = new uint32_t[96*27];
  text_pixels_ = new uint32_t[768*432];

  const auto flags = fullscreen
      ? SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN
      : SDL_WINDOW_ALLOW_HIGHDPI;
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    std::cerr << SDL_GetError() << std::endl;
    assert(false);
  }
  window_ = SDL_CreateWindow("GVM", SDL_WINDOWPOS_UNDEFINED,  SDL_WINDOWPOS_UNDEFINED,
      width, height, flags);
  assert(window_ != nullptr);
  SDL_GetWindowSize(window_, &maxW_, &maxH_);


  int drv_index = -1;
  if (!force_driver.empty()) {
    SDL_Log("Available renderers:\n");

    for(int it = 0; it < SDL_GetNumRenderDrivers(); it++) {
      SDL_RendererInfo info;
      SDL_GetRenderDriverInfo(it,&info);
      SDL_Log("%s\n", info.name);
      if (force_driver == info.name) {
        drv_index = it;
        SDL_Log("picked opengles2");
        break;
      }
    }
    if (drv_index == -1) {
      SDL_Log("%s was not available. Letting SDL choose.", force_driver.c_str());
    }
  }

  renderer_ = SDL_CreateRenderer(window_, drv_index, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer_ == nullptr) {
    SDL_Log("%s", SDL_GetError());
    std::cerr << "Hardware acceleration unavailable. Fallback to software.\n";
    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
  }
}

SDL2VideoDisplay::~SDL2VideoDisplay() {
  delete [] text_vram_buffer_;
  delete [] text_pixels_;
  SDL_DestroyTexture(texture_);
  SDL_DestroyTexture(text_texture_);
  SDL_DestroyRenderer(renderer_);
  SDL_DestroyWindow(window_);
  SDL_Quit();
}

void SDL2VideoDisplay::SetFramebufferSize(int fWidth, int fHeight, int bpp) {
  assert(fWidth <= maxW_);
  assert(fHeight <= maxH_);
  assert(bpp == 32);
  fWidth_ = fWidth;
  fHeight_ = fHeight;
  texture_ = SDL_CreateTexture(
      renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, fWidth, fHeight);
  assert(texture_ != nullptr);
  text_texture_ = SDL_CreateTexture(
      renderer_, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, 768, 432);
  assert(text_texture_ != nullptr);
}

void SDL2VideoDisplay::CopyBuffer(uint32_t* mem, uint32_t mode) {
  if (mode == 1) {
    int pitch;
    void* pixels = nullptr;
    if (SDL_LockTexture(texture_,  nullptr, &pixels, &pitch) != 0) {
      std::cerr << SDL_GetError() << "\n";
      assert(false);
    }
    assert(pixels != nullptr);
    std::memcpy(pixels, mem, pitch * fHeight_);
    SDL_UnlockTexture(texture_);
  } else if (mode == 2) {
    std::memcpy(text_vram_buffer_, mem, sizeof(uint32_t)*96*27);
  }
}

void SDL2VideoDisplay::Render(uint32_t mode) {
  if (SDL_RenderClear(renderer_) != 0) {
    std::cerr << "RendererClear: " << SDL_GetError() << std::endl;
  }

  if (mode == 1) {
    GraphicsRender();
  } else if (mode == 2) {
    TextRender();
  }

  SDL_RenderPresent(renderer_);
}

void SDL2VideoDisplay::GraphicsRender() {
  if (SDL_RenderCopy(renderer_, texture_, nullptr, nullptr) != 0) {
    if (count_ % 20000 == 0) {
      std::cerr << "RenderCopy: " << SDL_GetError() << std::endl;
    }
    ++count_;
  }
}

static void renderChar(
    uint32_t ch, uint32_t fg, uint32_t bg, int x, int y,
    uint32_t* text_rom,  uint32_t* vram_pixels) {
    const uint32_t line_size = 768;
    const uint32_t char_width = 8;
    uint32_t* char_word = &text_rom[ch << 2];
    uint32_t idx = y * line_size * 16 + x * char_width - line_size;
    for (int w = 0; w < 4; ++w) {
        uint32_t word = char_word[w];
        for (uint32_t i = 0; i < sizeof(uint32_t)*char_width; ++i) {
            if (i % 8 == 0) {
                idx += line_size + char_width;
            }
            auto c = (word >> i) & 0x01;
            vram_pixels[--idx] = c == 0 ? bg : fg;
        }
    }
}

void SDL2VideoDisplay::TextRender() {
  for (int y = 0; y < 27; ++y) {
    for (int x = 0; x < 96; ++x) {
      const auto i = y*96 + x;
      const auto ch = text_vram_buffer_[i] & 0xFFFF;
      const auto fg = kColorTable[(text_vram_buffer_[i] >> 16) & 0xFF];
      const auto bg = kColorTable[(text_vram_buffer_[i] >> 24) & 0xFF];
      renderChar(ch, fg, bg, x, y, text_rom_, text_pixels_);
    }
  }

  int pitch;
  void* pixels = nullptr;
  if (SDL_LockTexture(text_texture_,  nullptr, &pixels, &pitch) != 0) {
    std::cerr << SDL_GetError() << "\n";
    assert(false);
  }
  assert(pixels != nullptr);
  std::memcpy(pixels, text_pixels_, pitch * 432);
  SDL_UnlockTexture(text_texture_);

  if (SDL_RenderCopy(renderer_, text_texture_, nullptr, nullptr) != 0) {
    if (count_ % 20000 == 0) {
      std::cerr << "RenderCopy: " << SDL_GetError() << std::endl;
    }
    ++count_;
  }
}

bool SDL2VideoDisplay::CheckEvents() {
  return false;
}

}  // namespace gvm
