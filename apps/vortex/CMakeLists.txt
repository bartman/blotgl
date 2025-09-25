add_executable(vortex
        main.cpp
)

TARGET_COMPILE_DEFINITIONS(vortex PRIVATE
    FMT_HEADER_ONLY
)

TARGET_INCLUDE_DIRECTORIES(vortex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${clipp_SOURCE_DIR}/include
    ${BLOTGL_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(vortex PRIVATE
    blotgl_a
    -lm
    #spdlog::spdlog
    fmt::fmt
    EGL::EGL
    GBM::GBM
    OpenGL::GL
    glm::glm
)
