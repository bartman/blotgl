#pragma once
#include <vector>
#include <string>
#include "blotgl_utils.hpp"

extern std::vector<std::string> g_blotgl_glerrors;

extern void blotgl_check_glerror(const char* operation);
#define GL(code) ({ \
    code; \
    blotgl_check_glerror(__stringify(code)); \
})

extern void blotgl_push_glerror(const char* operation);
extern void blotgl_drain_glerrors();

#define GLP(code) ({ \
    code; \
    blotgl_push_glerror(__stringify(code)); \
})
