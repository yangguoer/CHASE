# Pipeline module (BLP)
add_library(pipeline
    include/BlockPipeline.h
    src/BlockPipeline.cpp
)

target_include_directories(pipeline PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(pipeline PUBLIC
    framework
    scheduler
    state
)
