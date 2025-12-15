cmake_minimum_required(VERSION 3.7)
project(NumDetection LANGUAGES CXX)

find_package(OpenCL REQUIRED)

set(PRODUCTION_BUILD False)

if (PRODUCTION_BUILD)
    set(EXE_NAME ${PROJECT_NAME})
else()
    set(EXE_NAME "out")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
add_executable(${EXE_NAME} main.cpp)

include_directories(
    "${CMAKE_SOURCE_DIR}/include/"
)

target_link_libraries(${EXE_NAME} PRIVATE OpenCL::OpenCL)

set_target_properties(${EXE_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)