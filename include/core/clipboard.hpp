#pragma once
#include <string>

namespace k2 {
class Clipboard {
    static std::string get();
    static void set(const std::string& str);
};
}  // namespace k2