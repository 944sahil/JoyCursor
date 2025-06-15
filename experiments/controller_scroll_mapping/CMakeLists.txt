cmake_minimum_required(VERSION 3.10)
project(ControllerScrollMapping)

# Find SDL2 package
find_package(SDL2 REQUIRED)

# Include directories
include_directories(${SDL2_INCLUDE_DIRS})

# Add source files
file(GLOB SOURCES "src/*.cpp")

# Create executable
add_executable(scroll_mapper ${SOURCES})

# Link libraries
target_link_libraries(scroll_mapper ${SDL2_LIBRARIES})

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Windows-specific configuration
if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,console")
endif() 