#pragma once
#include <cstdint>
#include <array>

namespace BlotGL {

inline auto div_round_up(auto n, auto by) { return (n + by - 1) / by; }

}
