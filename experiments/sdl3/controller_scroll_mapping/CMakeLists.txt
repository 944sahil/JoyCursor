cmake_minimum_required(VERSION 3.10)
project(controller_scroll_mapping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 package
find_package(SDL3 REQUIRED)

# Add executable
add_executable(scroll_mapper src/scroll_mapper.cpp)

# Link SDL3
target_link_libraries(scroll_mapper PRIVATE SDL3::SDL3) 