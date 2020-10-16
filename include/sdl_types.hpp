#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include <memory>
#include <type_traits>

namespace k2 {
template <auto T> struct generic_deleter {
  operator decltype(T)() const noexcept { return T; }
};

using Texture_ptr =
    std::unique_ptr<SDL_Texture, generic_deleter<SDL_DestroyTexture>>;
using Window_ptr =
    std::unique_ptr<SDL_Window, generic_deleter<SDL_DestroyWindow>>;
using Renderer_ptr =
    std::unique_ptr<SDL_Renderer, generic_deleter<SDL_DestroyRenderer>>;
using Surface_ptr =
    std::unique_ptr<SDL_Surface, generic_deleter<SDL_FreeSurface>>;
using TTF_ptr = std::unique_ptr<TTF_Font, generic_deleter<TTF_CloseFont>>;
} // namespace k2