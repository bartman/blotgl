add_executable(blotgl
        blotgl_main.cpp
)

target_include_directories(blotgl
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

TARGET_COMPILE_DEFINITIONS(blotgl PRIVATE
    FMT_HEADER_ONLY
)

TARGET_INCLUDE_DIRECTORIES(blotgl PRIVATE
    ${clipp_SOURCE_DIR}/include
)

TARGET_LINK_LIBRARIES(blotgl PRIVATE
    -lm
    spdlog::spdlog
    fmt::fmt
)
