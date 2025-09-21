#pragma once
#include <cstdint>

struct color24 {
    uint8_t r{}, g{}, b{};

    operator bool() const { return (r|g|b) != 0; }
    auto operator<=>(const color24&) const = default;
};
