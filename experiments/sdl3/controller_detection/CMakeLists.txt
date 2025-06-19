cmake_minimum_required(VERSION 3.10)
project(controller_detector)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 package
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# Add executable
add_executable(controller_detector src/controller_detector.cpp)

# Link SDL3
target_link_libraries(controller_detector PRIVATE SDL3::SDL3) 