#include "core/clipboard.hpp"

#include <GLFW/glfw3.h>
namespace k2 {

std::string Clipboard::get() { return { glfwGetClipboardString(nullptr) }; }
void Clipboard::set(const std::string& str) { glfwSetClipboardString(nullptr, str.c_str()); }

} // namespace k2
