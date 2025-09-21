#pragma once
#include <cstdint>
#include <array>

namespace BlotGL {

static const constexpr uint16_t BRAILLE_GLYPH_BASE = 0x2800;

static const constexpr size_t BRAILLE_GLYPH_COLS = 2;
static const constexpr size_t BRAILLE_GLYPH_ROWS = 4;
static const constexpr size_t BRAILLE_GLYPH_SIZE = (BRAILLE_GLYPH_ROWS * BRAILLE_GLYPH_COLS);

inline auto braille_glyph_map_index(auto x, auto y) { return y * BRAILLE_GLYPH_COLS + x; }

inline auto div_round_up(auto n, auto by) { return (n + by - 1) / by; }

// this maps the order of bits in a 2x4 image into braille character glyph locations
static const constexpr std::array<uint8_t,BRAILLE_GLYPH_SIZE> braille_mapping = {
        0x01, 0x08,
        0x02, 0x10,
        0x04, 0x20,
        0x40, 0x80,
    };

}
