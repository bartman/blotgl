include(FindPkgConfig)
INCLUDE(FetchContent)

function(my_find_package LIB_NAME TARGET_NAME)
  find_package(${LIB_NAME} QUIET)
  if(NOT TARGET ${TARGET_NAME})
    string(TOLOWER ${LIB_NAME} PKG_NAME)
    pkg_check_modules(${LIB_NAME}_PKGCONF REQUIRED ${PKG_NAME})
    find_library(${LIB_NAME}_LIBRARY NAMES ${${LIB_NAME}_PKGCONF_LIBRARIES} HINTS ${${LIB_NAME}_PKGCONF_LIBRARY_DIRS})
    if(NOT ${LIB_NAME}_LIBRARY)
      message(FATAL_ERROR "${LIB_NAME} library not found")
    endif()
    add_library(${TARGET_NAME} UNKNOWN IMPORTED)
    set_target_properties(${TARGET_NAME} PROPERTIES
      IMPORTED_LOCATION "${${LIB_NAME}_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${${LIB_NAME}_PKGCONF_INCLUDE_DIRS}"
    )
  endif()
endfunction()

# this uses system libraries, preferably via pkg-config
my_find_package(GTest GTest::gtest)
my_find_package(EGL EGL::EGL)
my_find_package(GBM GBM::GBM)
my_find_package(glm glm::glm)
my_find_package(OpenGL OpenGL::GL)
find_package(Threads REQUIRED)

# this downloads source and builds it
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
)
FetchContent_Declare(
    clipp
    GIT_REPOSITORY https://github.com/muellan/clipp.git
    GIT_TAG v1.2.3
)
FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.2.0
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Add this line

#FetchContent_MakeAvailable(spdlog clipp fmt)      # <=== not using spdlog or clipp yet
FetchContent_MakeAvailable(fmt)
