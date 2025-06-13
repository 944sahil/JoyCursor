cmake_minimum_required(VERSION 3.10)
project(ControllerDetection)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(controller_detector ${SOURCES})

# Link libraries
target_link_libraries(controller_detector ${SDL2_LIBRARIES})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) 