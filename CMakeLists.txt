cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 17)

project(MPM)

include_directories(include)
include_directories(external/glad/include)
include_directories(external/partio/src/lib)
include_directories(external/spdlog/include)

# aux_source_directory(src/MTR MTR_SRCS)
# aux_source_directory(src SRCS)
# set(SPECIFIC_OUTPUT_DIR "${CMAKE_BUILD_TYPE}")

set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin")

if(NOT MSVC)
    set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
    set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

message(${EXECUTABLE_OUTPUT_PATH})

add_subdirectory(external/glad)
add_subdirectory(external/partio)
add_subdirectory(external/spdlog)

add_subdirectory(src/MPM)