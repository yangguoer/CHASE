# State module (two-layer cache)
add_library(state
    include/StateCache.h
    src/StateCache.cpp
)

target_include_directories(state PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(state PUBLIC
    framework
)
