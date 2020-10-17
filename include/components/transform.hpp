#pragma once
#include <cstdint>

struct TransformComponent {
  std::int64_t x, y, z;
  float rotation, scale;
};