#pragma once
#include "core/app.hpp"
#include <memory>
#include <string>
#include <vector>

extern std::unique_ptr<k2::App> create_app(std::vector<std::string> args);

#ifdef __linux__
int main(int argc, char** argv, char**) { create_app({ argv + 1, argv + argc })->run(); }
#elif _WIN32
// Prefer the use of the high performance GPU
extern "C" {
__declspec(dllexport) uint32_t NvOptimusEnablement = 1;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int __stdcall WinMain(void*, void*, char*, int) {
    create_app({ __argv + 1, __argv + __argc })->run();
    return 0;
}
#elif __APPLE__
int main(int argc, char** argv) { create_app({ argv + 1, argv + argc })->run(); }
#else
#error "Your platform is not supported."
#endif
