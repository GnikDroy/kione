#include "entt/entt.hpp"

#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl.h"

#include "spdlog/spdlog.h"

#include "core/sdl_types.hpp"
#include <iostream>
#include <memory>
#include <string>

#include "core/app.hpp"

namespace k2
{
    App::App()
    {
        // if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        // {
        //     throw std::runtime_error("Cannot initialize SDL");
        // }

        // if (TTF_Init() < 0)
        // {
        //     throw std::runtime_error("Cannot initialize SDL_TTF");
        // }

        // int img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF | IMG_INIT_WEBP;
        // if ((IMG_Init(img_flags) & img_flags) != img_flags)
        // {
        //     throw std::runtime_error("Cannot initialize SDL_Image");
        // }

        // int mixer_flags = MIX_INIT_OGG | MIX_INIT_MOD | MIX_INIT_MP3 |
        //                   MIX_INIT_FLAC | MIX_INIT_OPUS;
        // if ((mixer_flags & Mix_Init(mixer_flags)) != mixer_flags)
        // {
        //     throw std::runtime_error("Cannot initialize SDL_Mixer");
        // }

        // confit.window_flags = SDL_WINDOW_SHOWN;
        // window.reset(SDL_CreateWindow(config.window_title.c_str(), config.window_x,
        //                               config.window_y, config.window_width,
        //                               config.window_height, config.window_flags));
        // if (!window)
        // {
        //     throw std::runtime_error("Window could not be created");
        // }
        // renderer.reset(
        //     SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED));
        // if (!renderer)
        // {
        //     throw std::runtime_error("Renderer could not be created");
        // }
        // SDL_RenderSetLogicalSize(renderer.get(), config.window_width,
        //                          config.window_height);
        // SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
    }

    App::~App()
    {
        // IMG_Quit();
        // Mix_Quit();
        // TTF_Quit();
        // SDL_Quit();
    }

    void App::run() {}
} // namespace k2