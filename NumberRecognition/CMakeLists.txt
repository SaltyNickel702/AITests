cmake_minimum_required(VERSION 4.2.1)
project(NumDetection LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCL REQUIRED)
find_package(OpenGL REQUIRED)

set(PRODUCTION_BUILD False)

if (PRODUCTION_BUILD)
    set(EXE_NAME ${PROJECT_NAME})
else()
    set(EXE_NAME "out")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

include_directories(
    "${CMAKE_SOURCE_DIR}/include/"
)
link_directories("${CMAKE_SOURCE_DIR}/lib")

file(GLOB_RECURSE SOURCES "${SRC_DIR}/*.cpp")
add_executable(${EXE_NAME} ${SOURCES} ${APP_ICON_RESOURCE_WINDOWS})

target_link_libraries(${EXE_NAME} 
    PRIVATE OpenCL::OpenCL
    opengl32
    glew32
    glfw3
)