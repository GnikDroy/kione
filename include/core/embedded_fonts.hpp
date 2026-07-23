#pragma once

#include <cstddef>

namespace k2::embedded_fonts {

// Editor UI fonts compiled into the binary (see cmake/embed_fonts.cmake).
// C arrays: the byte-blob size is only known at generation time.
extern const unsigned char NotoSans_Regular[]; // NOLINT(modernize-avoid-c-arrays)
extern const std::size_t NotoSans_Regular_size;
extern const unsigned char material_symbols_outlined[]; // NOLINT(modernize-avoid-c-arrays)
extern const std::size_t material_symbols_outlined_size;

}
