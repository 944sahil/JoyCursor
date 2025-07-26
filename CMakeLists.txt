cmake_minimum_required(VERSION 3.10)
project(JoyCursor VERSION 1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

# Set output directories BEFORE creating targets
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find SDL3
find_package(SDL3 REQUIRED)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Widgets)

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Add source files
file(GLOB_RECURSE CORE_SOURCES "src/core/*.cpp" "src/platform/*.cpp" "src/utils/*.cpp")
file(GLOB UI_SOURCES "src/ui/*.cpp" "src/ui/*.h" "src/workers/CoreWorker.cpp" "src/workers/CoreWorker.h")
list(REMOVE_ITEM UI_SOURCES ${CMAKE_SOURCE_DIR}/src/ui/ResourceTest.cpp)

# Add Qt resource file
qt6_add_resources(GENERATED_RESOURCES src/resources/resources.qrc)

# Main Qt UI executable
add_executable(JoyCursor src/main.cpp ${CORE_SOURCES} ${UI_SOURCES} ${GENERATED_RESOURCES})
target_link_libraries(JoyCursor PRIVATE SDL3::SDL3 Qt6::Widgets)
# if (WIN32)
#     set_target_properties(JoyCursor PROPERTIES WIN32_EXECUTABLE TRUE)
# endif()

# CLI core executable
add_executable(JoyCursorCore src/core_main.cpp ${CORE_SOURCES})
target_link_libraries(JoyCursorCore PRIVATE SDL3::SDL3)

# Copy resources to build directory
add_custom_command(TARGET JoyCursor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/src/resources/mappings.json
    ${CMAKE_BINARY_DIR}/bin/mappings.json
)