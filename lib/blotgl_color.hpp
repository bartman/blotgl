#pragma once
#include <cstdint>
#include <compare>

struct color24 {
    uint8_t r{}, g{}, b{};

    operator bool() const { return (r|g|b) != 0; }
    auto operator<=>(const color24&) const = default;
};

struct color64 {
    uint16_t r{}, g{}, b{};

    operator bool() const { return (r|g|b) != 0; }
    auto operator<=>(const color64&) const = default;
};
