#pragma once
#include <vector>
#include <string>
#include "blotgl_utils.hpp"

extern std::vector<std::string> g_blotgl_glerrors;

extern void blotgl_push_glerror(const char* operation);
extern size_t blotgl_drain_glerrors();

#define GL(code) ({ \
    code; \
    blotgl_push_glerror(__stringify(code)); \
})
