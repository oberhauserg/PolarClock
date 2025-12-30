cmake_minimum_required(VERSION 3.16)
project(PolarClock VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Source files
set(SOURCES
    src/main.cpp
    src/shader.cpp
    src/renderer.cpp
    src/arc_renderer.cpp
    src/polar_clock.cpp
    src/text_renderer.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty
)

if(EMSCRIPTEN)
    # Emscripten build settings
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    # Emscripten link-only flags
    set(EM_LINK_FLAGS
        "-s USE_GLFW=3"
        "-s USE_WEBGL2=1"
        "-s FULL_ES3=1"
        "-s WASM=1"
        "-s INITIAL_MEMORY=16777216"
        "-s ALLOW_MEMORY_GROWTH=1"
        "-s NO_EXIT_RUNTIME=1"
        "-s MIN_WEBGL_VERSION=2"
        "-s MAX_WEBGL_VERSION=2"
        "--preload-file ${CMAKE_SOURCE_DIR}/assets@/assets"
        "--preload-file ${CMAKE_SOURCE_DIR}/shaders@/shaders"
        "--shell-file ${CMAKE_SOURCE_DIR}/web/shell.html"
    )

    # Join flags for linking only
    string(JOIN " " EM_LINK_FLAGS_STR ${EM_LINK_FLAGS})

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "${EM_LINK_FLAGS_STR}"
    )

    message(STATUS "Building for Emscripten (WebAssembly)")
else()
    # Native build - find dependencies
    find_package(glfw3 3.3 REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(GLEW REQUIRED)

    target_link_libraries(${PROJECT_NAME} PRIVATE
        glfw
        OpenGL::GL
        GLEW::GLEW
    )

    # Copy assets and shaders to build directory for native testing
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/shaders
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
    )

    message(STATUS "Building for native platform")
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
