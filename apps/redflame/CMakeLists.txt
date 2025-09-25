add_executable(redflame
        main.cpp
)

TARGET_COMPILE_DEFINITIONS(redflame PRIVATE
    FMT_HEADER_ONLY
)

TARGET_INCLUDE_DIRECTORIES(redflame PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${clipp_SOURCE_DIR}/include
    ${BLOTGL_SOURCE_DIR}
)

TARGET_LINK_LIBRARIES(redflame PRIVATE
    blotgl_a
    -lm
    #spdlog::spdlog
    fmt::fmt
    EGL::EGL
    GBM::GBM
    OpenGL::GL
    glm::glm
)
