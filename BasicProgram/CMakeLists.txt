cmake_minimum_required(VERSION 4.2.1)
project(NumDetection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
add_executable(${EXE_NAME} ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS})


include_directories(
    "${CMAKE_SOURCE_DIR}/include/"
)

target_link_libraries(${EXE_NAME} PRIVATE OpenCL::OpenCL)