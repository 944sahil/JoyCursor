cmake_minimum_required(VERSION 3.10)
project(ControllerMouseMapping)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executables
add_executable(mouse_mapper src/mouse_mapper.cpp)
add_executable(controller_restore src/controller_restore.cpp)

# Link libraries
target_link_libraries(mouse_mapper ${SDL2_LIBRARIES})
target_link_libraries(controller_restore ${SDL2_LIBRARIES})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows-specific configuration
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,console")
endif() 