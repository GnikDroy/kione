#pragma once
#include <memory>

#include <sol/forward.hpp>

#include "asset/asset_handle.hpp"

namespace k2 {
using ScriptEnvironment = std::unique_ptr<sol::environment, void (*)(sol::environment*)>;

struct ScriptComponent {
    AssetHandle script {};
    ScriptEnvironment env { nullptr, nullptr };
};
}
