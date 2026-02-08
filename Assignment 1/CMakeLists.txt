cmake_minimum_required(VERSION 3.10)
project(PlaneRotation)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/headers
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/include
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui
)

# Add GLAD source
set(GLAD_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/src/glad.c
)

# Add ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/imgui/imgui_impl_opengl3.cpp
)

# Add main source files
set(MAIN_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/stb.cpp
)

# Create executable
add_executable(PlaneRotation 
    ${MAIN_SOURCES}
    ${GLAD_SOURCES}
    ${IMGUI_SOURCES}
)

# Link libraries
target_link_libraries(PlaneRotation
    ${OPENGL_LIBRARIES}
    glfw
    assimp::assimp
)

# Copy shader and asset directories to build directory
add_custom_command(TARGET PlaneRotation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders
    ${CMAKE_BINARY_DIR}/bin/shaders
    COMMENT "Copying shaders..."
)

add_custom_command(TARGET PlaneRotation POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/assets
    ${CMAKE_BINARY_DIR}/bin/assets
    COMMENT "Copying assets..."
)

# Set working directory to bin for proper shader/asset loading
set_target_properties(PlaneRotation PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Print summary
message(STATUS "OpenGL Project Configuration: PlaneRotation")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output directory: ${CMAKE_BINARY_DIR}/bin")
