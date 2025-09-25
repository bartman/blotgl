#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <fmt/core.h>
#include <fmt/ostream.h>

#include "app.hpp"

int main() {
    BlotGL::App app;

    app.push<AppLayer>();

    app.run();

    return 0;
}
