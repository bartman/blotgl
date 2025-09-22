#pragma once

#include "blotgl_braille.hpp"
#include "blotgl_utils.hpp"
#include "blotgl_color.hpp"

namespace BlotGL {

template <size_t BPP = 3>
class Frame final {
public:
    using Size = uint32_t;

    Frame(Size width, Size height)
    : m_width(width), m_height(height),
      m_pixels(pixel_size() * BPP, 0),
      m_braille(braille_size(), 0),
      m_colors(braille_size(), color24{})
    {
        static_assert(BPP == 3); // only this is supported for now
    }
    ~Frame() = default;

    // RGB pixel buffer

    Size pixel_width() const { return m_width; }
    Size pixel_height() const { return m_height; }
    size_t pixel_size() const { return size_t(pixel_width()) * size_t(pixel_height()); }
    uint8_t* pixels() { return m_pixels.data(); }
    uint8_t* pixel_ptr(Size x, Size y) {
        size_t index = pixel_index(x, y);
        return pixels() + (index * BPP);
    }
    color24 pixel_color(Size x, Size y) {
        const uint8_t *rgb = pixel_ptr(x, y);
        return { rgb[0], rgb[1], rgb[2] };
    }

    void pixel_reset() {
        std::fill(m_pixels.begin(), m_pixels.end(), 0);
    }

    // braille glyph buffer

    Size braille_width() const { return div_round_up(m_width, BlotGL::BRAILLE_GLYPH_COLS); }
    Size braille_height() const { return div_round_up(m_height, BlotGL::BRAILLE_GLYPH_ROWS); }
    size_t braille_size() const { return size_t(braille_width()) * size_t(braille_height()); }
    uint8_t* braille() { return m_braille.data(); }
    uint8_t& braille(Size x, Size y) {
        size_t index = braille_index(x, y);
        return braille()[index];
    }

    void braille_reset() {
        std::fill(m_braille.begin(), m_braille.end(), 0);
    }

    // color buffer (same size as braille character buffer)

    color24* colors() { return m_colors.data(); }
    color24& color(Size x, Size y) {
        size_t index = braille_index(x, y);
        return colors()[index];
    }

    void color_reset() {
        std::fill(m_colors.begin(), m_colors.end(), color24{});
    }

    void reset() {
        pixel_reset();
        braille_reset();
        color_reset();
    }

    // convert pixel buffer to braille/colors
    void pixels_to_braille() {
        for (size_t y=0; y<pixel_height(); y++) {
            for (size_t x=0; x<pixel_width(); x++) {
                color24 pix = pixel_color(x,y);
                if (!pix)
                    continue;

                // locate in braille array
                Size bx = x/BRAILLE_GLYPH_COLS;
                Size by = y/BRAILLE_GLYPH_ROWS;
                auto &b = braille(bx, by);
                auto &c = color(bx, by);

                // locate within braille 2x4 glyph
                Size gx = x%BlotGL::BRAILLE_GLYPH_COLS;
                Size gy = y%BlotGL::BRAILLE_GLYPH_ROWS;
                uint8_t g = braille_glyph_at(gx, gy);

                b += g;
                c = pix;
            }
        }
    }

    std::ostream & braille_to_stream(std::ostream &out) {
        gen_clear_screen(out);
        gen_top_left(out);
        for (size_t y=0; y<braille_height(); y++) {
            color24 prev_color{};
            for (size_t x=0; x<braille_width(); x++) {
                uint8_t g = braille(x, y);
                if (!g) {
                    out << ' ';
                    continue;
                }

                color24 c = color(x, y);
                if (prev_color != c) {
                    prev_color = c;
                    gen_color(out, c);
                }
                gen_unicode(out, BRAILLE_GLYPH_BASE + g);
            }
            if (prev_color)
                gen_reset(out);

            out << '\n';
        }
        return out;
    }


protected:
    const Size m_width;
    const Size m_height;
    std::vector<uint8_t> m_pixels;
    std::vector<uint8_t> m_braille;
    std::vector<color24> m_colors;

    static constexpr size_t buffer_size(size_t width, size_t height) {
        return width * height * BPP;
    }
    constexpr size_t pixel_index(size_t x, size_t y) {
        assert (x < pixel_width());
        assert (y < pixel_height());
        return x + (y * pixel_width());
    }
    constexpr size_t braille_index(size_t x, size_t y) {
        assert (x < braille_width());
        assert (y < braille_height());
        return x + (y * braille_width());
    }
    static constexpr uint8_t braille_glyph_at(uint8_t x, uint8_t y) {
        assert (x < BRAILLE_GLYPH_COLS);
        assert (y < BRAILLE_GLYPH_ROWS);
        size_t index = x + (y*BRAILLE_GLYPH_COLS);
        return BlotGL::braille_mapping[index];
    }
    static constexpr void gen_unicode(std::ostream &out, char32_t codepoint) {
        if (codepoint <= 0x7F) {
            out << char(codepoint);
        } else if (codepoint <= 0x7FF) {
            out << char(0xC0 | ((codepoint >> 6) & 0x1F))
                << char(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0xFFFF) {
            out << char(0xE0 | ((codepoint >> 12) & 0x0F))
                << char(0x80 | ((codepoint >> 6) & 0x3F))
                << char(0x80 | (codepoint & 0x3F));
        } else if (codepoint <= 0x10FFFF) {
            out << char(0xF0 | ((codepoint >> 18) & 0x07))
                << char(0x80 | ((codepoint >> 12) & 0x3F))
                << char(0x80 | ((codepoint >> 6) & 0x3F))
                << char(0x80 | (codepoint & 0x3F));
        }
    };

    static constexpr void gen_color(std::ostream &out, color24 color) {
        out << std::format("\033[38;2;{};{};{}m", color.r, color.g, color.b);
    };
    static constexpr void gen_reset(std::ostream &out) {
        out << "\033[0m";
    }
    static constexpr void gen_clear_screen(std::ostream &out) {
        out << "\033[2J";
    }
    static constexpr void gen_top_left(std::ostream &out) {
        out << "\033[H";
    }
};

}
