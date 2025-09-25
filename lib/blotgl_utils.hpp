#pragma once
#include <cstdint>
#include <array>

#define __stringifyx(x) #x
#define __stringify(x) __stringifyx(x)

namespace BlotGL {

inline auto div_round_up(auto n, auto by) { return (n + by - 1) / by; }

template <typename T>
inline T multiple_of(T n, T m) { return (n/m) * m; }

}
