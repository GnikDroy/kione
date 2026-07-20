#include "core/paths.hpp"

#include <string>

#if defined(__APPLE__)
#include <cstdint>
#include <mach-o/dyld.h>
#elif defined(_WIN32)
#include <array>
#include <windows.h>
#endif

namespace k2 {

std::filesystem::path executable_path() {
#if defined(__APPLE__)
    std::uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);
    std::string buffer(size, '\0');
    _NSGetExecutablePath(buffer.data(), &size);
    return std::filesystem::canonical(buffer.c_str());
#elif defined(_WIN32)
    std::array<wchar_t, MAX_PATH> buffer {};
    GetModuleFileNameW(nullptr, buffer.data(), DWORD(buffer.size()));
    return std::filesystem::canonical(buffer.data());
#else
    return std::filesystem::canonical("/proc/self/exe");
#endif
}

}
