cmake_minimum_required(VERSION 3.11)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Specify the C++ compiler to be Clang
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")
include(PickCompiler)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable generation of compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(blot_gl_test VERSION 0.0.0 LANGUAGES C CXX)

include(GetGitRevisionDescription)
git_describe(GIT_REVISION --tags --dirty=-dirty)
message(STATUS "Git revision ${GIT_REVISION}")

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-march=native)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(-fsanitize=address)
        add_link_options(-fsanitize=address)
    endif()
endif()

include(CheckLibCppHeaders)

include(SetupDependencies)

# Add the src subdirectory
add_subdirectory(src)

