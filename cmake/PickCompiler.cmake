# Function to find the highest version of a compiler (clang or clang++)
function(find_highest_clang_compiler compiler_name output_variable)
    set(compiler_versions
        23 22 21 20 19 18 17 16 15 14 13 12 11 10
    )
    set(compiler_path "")

    # Check for versioned compilers (e.g., clang-18, clang++-18)
    foreach(version ${compiler_versions})
        find_program(${compiler_name}_VERSION_${version}
            NAMES "${compiler_name}-${version}"
            PATHS ENV PATH
            NO_DEFAULT_PATH
        )
        if(${compiler_name}_VERSION_${version})
            set(compiler_path "${${compiler_name}_VERSION_${version}}")
            break()
        endif()
    endforeach()

    # Fallback to unversioned compiler (e.g., clang, clang++)
    if(NOT compiler_path)
        find_program(${compiler_name}_DEFAULT
            NAMES "${compiler_name}"
            PATHS ENV PATH
            NO_DEFAULT_PATH
        )
        set(compiler_path "${${compiler_name}_DEFAULT}")
    endif()

    # Set the output variable to the found compiler path
    set(${output_variable} "${compiler_path}" PARENT_SCOPE)

    # Clear cached variables to avoid stale results
    foreach(version ${compiler_versions})
        unset(${compiler_name}_VERSION_${version} CACHE)
    endforeach()
    unset(${compiler_name}_DEFAULT CACHE)
endfunction()

# Find the highest version of clang and clang++
find_highest_clang_compiler("clang" CMAKE_C_COMPILER)
find_highest_clang_compiler("clang++" CMAKE_CXX_COMPILER)

# Verify that compilers were found
if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "No suitable Clang compiler found (clang-<version> or clang)")
endif()
if(NOT CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "No suitable Clang++ compiler found (clang++-<version> or clang++)")
endif()

# Set the compiler variables
set(CMAKE_C_COMPILER "${CMAKE_C_COMPILER}" CACHE PATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}" CACHE PATH "C++ compiler" FORCE)

# Optional: Print the selected compilers
message(STATUS "Selected C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Selected C++ compiler: ${CMAKE_CXX_COMPILER}")
