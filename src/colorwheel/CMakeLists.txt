add_executable(colorwheel
        main.cpp
)

TARGET_COMPILE_DEFINITIONS(colorwheel PRIVATE
    FMT_HEADER_ONLY
)

TARGET_INCLUDE_DIRECTORIES(colorwheel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${clipp_SOURCE_DIR}/include
    ${BLOTGL_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(colorwheel PRIVATE
    -lm
    #spdlog::spdlog
    fmt::fmt
    EGL::EGL
    GBM::GBM
    OpenGL::GL
)
