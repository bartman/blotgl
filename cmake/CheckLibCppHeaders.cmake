include(CheckCXXSourceCompiles)

# ------------------------------------------------------------------------
# Test for <version> header
check_cxx_source_compiles("
    #include <version>
    #ifndef __cpp_lib_concepts
    #error \"__cpp_lib_concepts not defined\"
    #endif
    int main() { return 0; }
" HAVE_VERSION_HEADER)

if(NOT HAVE_VERSION_HEADER)
    message(WARNING "C++20 <version> header is not available; using fallback")
    add_definitions(-DNO_VERSION_HEADER)
else()
    message(STATUS "C++20 <version> header is available")
    add_definitions(-DHAVE_VERSION_HEADER)
endif()

# ------------------------------------------------------------------------
# Test for <ranges> header
check_cxx_source_compiles("
    #include <ranges>
    #include <map>
    int main() {
        std::map<int,int> m;
        auto it = std::ranges::find_if(m, [](const auto& pair) { return pair.first == 0; });
        return it != m.end();
    }
" HAVE_RANGES_HEADER)

if(NOT HAVE_RANGES_HEADER)
    message(WARNING "C++20 <ranges> header is not available; using fallback")
    add_definitions(-DNO_RANGES_HEADER)
else()
    message(STATUS "C++20 <ranges> header is available")
    add_definitions(-DHAVE_RANGES_HEADER)
endif()

# ------------------------------------------------------------------------
# Test for std::integral
check_cxx_source_compiles("
    #include <concepts>
    template <std::integral T>
    T add(T a, T b) { return a + b; }
    int main() { return add(5, 3); }
" HAVE_CPP_CONCEPTS)

if(NOT HAVE_CPP_CONCEPTS)
    message(WARNING "Compiler does not support std::integral; using fallback")
    add_definitions(-DNO_CONCEPTS)
else()
    message(STATUS "Compiler supports std::integral")
    add_definitions(-DHAVE_CONCEPTS)
endif()

# ------------------------------------------------------------------------
# Test if std::unordered_map has contains
check_cxx_source_compiles("
    #include <unordered_map>
    int main() {
        std::unordered_map<int, int> m;
        m.insert({1,2});
        return m.contains(1);
    }
" HAVE_UNORDERED_MAP_CONTAINS)

if(NOT HAVE_UNORDERED_MAP_CONTAINS)
    message(WARNING "std::unordered_map does not support contains(); using fallback")
    add_definitions(-DNO_UNORDERED_MAP_CONTAINS)
else()
    message(STATUS "std::unordered_map does support contains()")
    add_definitions(-DHAVE_UNORDERED_MAP_CONTAINS)
endif()
