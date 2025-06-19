cmake_minimum_required(VERSION 3.10)
project(controller_keyboard_mapping)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 package
find_package(SDL3 REQUIRED)

# Add source files
add_executable(keyboard_mapper src/keyboard_mapper.cpp)

# Link SDL3 libraries
target_link_libraries(keyboard_mapper PRIVATE SDL3::SDL3)

# For Windows, copy SDL3 DLL to output directory
if(WIN32)
    add_custom_command(TARGET keyboard_mapper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:keyboard_mapper>)
endif() 