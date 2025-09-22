#define GL_GLEXT_PROTOTYPES
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <compare>
#include <fmt/core.h>
#include <fmt/ostream.h>
#include <stdexcept>
#include <sstream>

extern "C" {
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>
#include <fcntl.h>
#include <unistd.h>
};

#include "blotgl_frame.hpp"
#include "blotgl_app.hpp"

int main() {
    BlotGL::App app(200, 100);
    app.frame();
    app.frame();
    return 0;
}
