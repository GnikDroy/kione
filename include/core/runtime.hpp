#pragma once

#include "core/audio_system.hpp"
#include "core/resources.hpp"
#include "core/script_system.hpp"

namespace k2 {
class Window;

struct Runtime {
    ResourceManager resources;
    AudioSystem audio;
    ScriptSystem scripts;

    explicit Runtime(Window& window)
        : scripts { window } { }
};
}
