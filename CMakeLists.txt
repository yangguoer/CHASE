cmake_minimum_required(VERSION 3.18)
project(CHASE VERSION 1.0.0 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-g")

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(PkgConfig REQUIRED)
pkg_check_modules(YAML_CPP REQUIRED yaml-cpp)
find_package(glog REQUIRED)
find_package(Protobuf REQUIRED)

# Generate Protocol Buffers files
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto_gen)
set(PROTOBUF_IMPORT_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/proto)
file(GLOB PROTO_FILES "${CMAKE_CURRENT_SOURCE_DIR}/proto/*.proto")

find_program(PROTOC_EXECUTABLE protoc)
if(PROTOC_EXECUTABLE AND PROTO_FILES)
    foreach(PROTO_FILE ${PROTO_FILES})
        get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)
        set(PROTO_OUTPUT_FILE "${CMAKE_CURRENT_BINARY_DIR}/proto_gen/${PROTO_NAME}.pb.h")
        add_custom_command(
            OUTPUT ${PROTO_OUTPUT_FILE}
            COMMAND ${PROTOC_EXECUTABLE}
            ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}/proto_gen -I${PROTOBUF_IMPORT_DIRS} ${PROTO_FILE}
            DEPENDS ${PROTO_FILE}
            COMMENT "Generating ${PROTO_NAME}.pb.h"
            VERBATIM
        )
        list(APPEND PROTO_GEN_FILES ${PROTO_OUTPUT_FILE})
    endforeach()
    
    add_custom_target(generate_proto ALL DEPENDS ${PROTO_GEN_FILES})
    
    # Add proto_gen directory to include directories for all targets
    include_directories(${CMAKE_CURRENT_BINARY_DIR}/proto_gen)
endif()

# Add subdirectories
add_subdirectory(crypto)
add_subdirectory(framework)
add_subdirectory(storage)
add_subdirectory(network)
add_subdirectory(txpool)
add_subdirectory(consensus)
add_subdirectory(scheduler)
add_subdirectory(executor)
add_subdirectory(state)
add_subdirectory(pipeline)

# Main executable
add_executable(CHASE_node 
    main.cpp
)

# Link libraries
target_link_libraries(CHASE_node
    PRIVATE
        framework
        crypto
        storage
        network
        txpool
        consensus
        scheduler
        executor
        state
        pipeline
        glog::glog
        OpenSSL::SSL
        OpenSSL::Crypto
        Boost::system
        Boost::filesystem
        Boost::thread
        protobuf::libprotobuf
        ${YAML_CPP_LIBRARIES}
)

# Include directories
target_include_directories(CHASE_node PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/proto_gen
)

# Global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/proto_gen
)