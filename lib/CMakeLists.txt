SET(BLOTGL_SRCS
    blotgl_app.cpp
)

# build a libblotgl.so and a libblotgl.a

ADD_LIBRARY(blotgl_a STATIC ${BLOTGL_SRCS})
ADD_LIBRARY(blotgl_so SHARED ${BLOTGL_SRCS})

# each is configured exactly the same way

foreach(blotgl blotgl_a blotgl_so)

    SET_TARGET_PROPERTIES(${blotgl} PROPERTIES OUTPUT_NAME blotgl)

    SET_PROPERTY(TARGET ${blotgl} PROPERTY POSITION_INDEPENDENT_CODE ON)

    TARGET_INCLUDE_DIRECTORIES(${blotgl} INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    TARGET_INCLUDE_DIRECTORIES(${blotgl} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    TARGET_INCLUDE_DIRECTORIES(${blotgl} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${clipp_SOURCE_DIR}/include
    )

    #TARGET_LINK_DIRECTORIES(${blotgl} PUBLIC
    #    EGL::EGL
    #    GBM::GBM
    #    OpenGL::GL
    #)

    TARGET_LINK_LIBRARIES(${blotgl} PRIVATE
        -lm
        #spdlog::spdlog
        fmt::fmt
        EGL::EGL
        GBM::GBM
        OpenGL::GL
    )

endforeach(blotgl)
