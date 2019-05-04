#include "sdl2_video_display.h"

#include <cassert>
#include <cstring>
#include <iostream>

namespace {

// XTerm 256 color palette.
static const uint32_t kColorTable[256] = {
	0xFF000000, 0xFF000008, 0xFF000800, 0xFF000808, 0xFF080000, 0xFF080008, 0xFF080800, 0xFF0C0C0C,
	0xFF080808, 0xFF0000FF, 0xFF00FF00, 0xFF00FFFF, 0xFFFF0000, 0xFFFF00FF, 0xFFFFFF00, 0xFFFFFFFF,
	0xFF000000, 0xFFF50000, 0xFF780000, 0xFFFA0000, 0xFF7D0000, 0xFFFF0000, 0xFF00F500, 0xFFF5F500,
	0xFF78F500, 0xFFFAF500, 0xFF7DF500, 0xFFFFF500, 0xFF007800, 0xFFF57800, 0xFF787800, 0xFFFA7800,
	0xFF7D7800, 0xFFFF7800, 0xFF00FA00, 0xFFF5FA00, 0xFF78FA00, 0xFFFAFA00, 0xFF7DFA00, 0xFFFFFA00,
	0xFF007D00, 0xFFF57D00, 0xFF787D00, 0xFFFA7D00, 0xFF7D7D00, 0xFFFF7D00, 0xFF00FF00, 0xFFF5FF00,
	0xFF78FF00, 0xFFFAFF00, 0xFF7DFF00, 0xFFFFFF00, 0xFF0000F5, 0xFFF500F5, 0xFF7800F5, 0xFFFA00F5,
	0xFF7D00F5, 0xFFFF00F5, 0xFF00F5F5, 0xFFF5F5F5, 0xFF78F5F5, 0xFFFAF5F5, 0xFF7DF5F5, 0xFFFFF5F5,
	0xFF0078F5, 0xFFF578F5, 0xFF7878F5, 0xFFFA78F5, 0xFF7D78F5, 0xFFFF78F5, 0xFF00FAF5, 0xFFF5FAF5,
	0xFF78FAF5, 0xFFFAFAF5, 0xFF7DFAF5, 0xFFFFFAF5, 0xFF007DF5, 0xFFF57DF5, 0xFF787DF5, 0xFFFA7DF5,
	0xFF7D7DF5, 0xFFFF7DF5, 0xFF00FFF5, 0xFFF5FFF5, 0xFF78FFF5, 0xFFFAFFF5, 0xFF7DFFF5, 0xFFFFFFF5,
	0xFF000078, 0xFFF50078, 0xFF780078, 0xFFFA0078, 0xFF7D0078, 0xFFFF0078, 0xFF00F578, 0xFFF5F578,
	0xFF78F578, 0xFFFAF578, 0xFF7DF578, 0xFFFFF578, 0xFF007878, 0xFFF57878, 0xFF787878, 0xFFFA7878,
	0xFF7D7878, 0xFFFF7878, 0xFF00FA78, 0xFFF5FA78, 0xFF78FA78, 0xFFFAFA78, 0xFF7DFA78, 0xFFFFFA78,
	0xFF007D78, 0xFFF57D78, 0xFF787D78, 0xFFFA7D78, 0xFF7D7D78, 0xFFFF7D78, 0xFF00FF78, 0xFFF5FF78,
	0xFF78FF78, 0xFFFAFF78, 0xFF7DFF78, 0xFFFFFF78, 0xFF0000FA, 0xFFF500FA, 0xFF7800FA, 0xFFFA00FA,
	0xFF7D00FA, 0xFFFF00FA, 0xFF00F5FA, 0xFFF5F5FA, 0xFF78F5FA, 0xFFFAF5FA, 0xFF7DF5FA, 0xFFFFF5FA,
	0xFF0078FA, 0xFFF578FA, 0xFF7878FA, 0xFFFA78FA, 0xFF7D78FA, 0xFFFF78FA, 0xFF00FAFA, 0xFFF5FAFA,
	0xFF78FAFA, 0xFFFAFAFA, 0xFF7DFAFA, 0xFFFFFAFA, 0xFF007DFA, 0xFFF57DFA, 0xFF787DFA, 0xFFFA7DFA,
	0xFF7D7DFA, 0xFFFF7DFA, 0xFF00FFFA, 0xFFF5FFFA, 0xFF78FFFA, 0xFFFAFFFA, 0xFF7DFFFA, 0xFFFFFFFA,
	0xFF00007D, 0xFFF5007D, 0xFF78007D, 0xFFFA007D, 0xFF7D007D, 0xFFFF007D, 0xFF00F57D, 0xFFF5F57D,
	0xFF78F57D, 0xFFFAF57D, 0xFF7DF57D, 0xFFFFF57D, 0xFF00787D, 0xFFF5787D, 0xFF78787D, 0xFFFA787D,
	0xFF7D787D, 0xFFFF787D, 0xFF00FA7D, 0xFFF5FA7D, 0xFF78FA7D, 0xFFFAFA7D, 0xFF7DFA7D, 0xFFFFFA7D,
	0xFF007D7D, 0xFFF57D7D, 0xFF787D7D, 0xFFFA7D7D, 0xFF7D7D7D, 0xFFFF7D7D, 0xFF00FF7D, 0xFFF5FF7D,
	0xFF78FF7D, 0xFFFAFF7D, 0xFF7DFF7D, 0xFFFFFF7D, 0xFF0000FF, 0xFFF500FF, 0xFF7800FF, 0xFFFA00FF,
	0xFF7D00FF, 0xFFFF00FF, 0xFF00F5FF, 0xFFF5F5FF, 0xFF78F5FF, 0xFFFAF5FF, 0xFF7DF5FF, 0xFFFFF5FF,
	0xFF0078FF, 0xFFF578FF, 0xFF7878FF, 0xFFFA78FF, 0xFF7D78FF, 0xFFFF78FF, 0xFF00FAFF, 0xFFF5FAFF,
	0xFF78FAFF, 0xFFFAFAFF, 0xFF7DFAFF, 0xFFFFFAFF, 0xFF007DFF, 0xFFF57DFF, 0xFF787DFF, 0xFFFA7DFF,
	0xFF7D7DFF, 0xFFFF7DFF, 0xFF00FFFF, 0xFFF5FFFF, 0xFF78FFFF, 0xFFFAFFFF, 0xFF7DFFFF, 0xFFFFFFFF,
	0xFF808080, 0xFF212121, 0xFFC1C1C1, 0xFF626262, 0xFF030303, 0xFFA3A3A3, 0xFF444444, 0xFFE4E4E4,
	0xFF858585, 0xFF262626, 0xFFC6C6C6, 0xFF676767, 0xFF080808, 0xFFA8A8A8, 0xFF494949, 0xFFE9E9E9,
	0xFF8A8A8A, 0xFF2B2B2B, 0xFFCBCBCB, 0xFF6C6C6C, 0xFF0D0D0D, 0xFFADADAD, 0xFF4E4E4E, 0xFFEEEEEE
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
    uint32_t* vram_pixels) {

}

void SDL2VideoDisplay::TextRender() {
  for (int y = 0; y < 27; ++y) {
    for (int x = 0; x < 96; ++x) {
      const auto i = y*27 + x*96;
      const auto ch = text_vram_buffer_[i] & 0xFFFF;
      const auto fg = kColorTable[(text_vram_buffer_[i] >> 16) & 0xFF];
      const auto bg = kColorTable[(text_vram_buffer_[i] >> 24) & 0xFF];
      renderChar(ch, fg, bg, x, y, text_pixels_);
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
