#pragma once
#include "bgfx/bgfx.h"
#include "bgfx/platform.h"
#include "core/window.hpp"


void activate_renderer(const k2::Window& window){
    bgfx::PlatformData pd;
    pd.nwh = window.get_native_handle();
    pd.ndt = window.get_native_display();
    pd.context = NULL;
    pd.backBuffer = NULL;
    pd.backBufferDS = NULL;

    bgfx::Init bgfxInit;
    bgfxInit.type = bgfx::RendererType::Count;  // Automatically choose a renderer.
    bgfxInit.resolution.width = window.get_width();
    bgfxInit.platformData = pd;
    bgfxInit.resolution.height = window.get_height();
    bgfxInit.resolution.reset = BGFX_RESET_VSYNC;
    bgfx::init(bgfxInit);
}
