#include "core/resources.hpp"

namespace k2::editor {
// This ensures that k2::editor::Resources is a unique type. Hence will store stuff in a unique singleton instance.
class EditorResourcesTag;
using Resources = BasicResources<Image, Texture2D, EditorResourcesTag*>;
// Something like BasicResources<...., decltype([](){})>; would be nice
// but since the header can be included multiple times, that would create different type declarations
// unless you only include this header once, or include a cpp file here, not a viable option.

}