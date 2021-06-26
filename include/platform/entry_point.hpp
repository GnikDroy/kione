#pragma once
#include "core/app.hpp"
#include <memory>

extern std::unique_ptr<k2::App> create_app();

#ifdef __linux__
int main(int, char**, char**) { create_app()->run(); }
#elif _WIN32
int __stdcall WinMain(void*, void*, char*, int) {
    create_app()->run();
    return 0;
}
#else
#error "Your platform is not supported."
#endif