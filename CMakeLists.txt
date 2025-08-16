cmake_minimum_required(VERSION 3.10)
project(notZelda)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# Output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Core sources from src/ and subdirectories
file(GLOB_RECURSE SRC_FILES src/*.cpp)

# ImGui core sources
set(IMGUI_DIR libs/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
)

# ImGui-SFML binding sources
set(IMGUI_SFML_DIR libs/imgui)
set(IMGUI_SFML_SOURCES
    ${IMGUI_SFML_DIR}/imgui-SFML.cpp
)

# Create executable
add_executable(notZelda
    ${SRC_FILES}
    ${IMGUI_SOURCES}
    ${IMGUI_SFML_SOURCES}
)

# Include directories
target_include_directories(notZelda PRIVATE
    include
    ${IMGUI_DIR}
    ${IMGUI_SFML_DIR}
)

# Find SFML and OpenGL
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)
find_package(OpenGL REQUIRED)

# Link everything
target_link_libraries(notZelda
    sfml-graphics
    sfml-window
    sfml-system
    sfml-audio
    OpenGL::GL
)


# Run target
add_custom_target(run
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/notZelda
    DEPENDS notZelda
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
