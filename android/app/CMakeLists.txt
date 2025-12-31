cmake_minimum_required(VERSION 3.22.1)
project(PolarClock VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Path to main source directory
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
set(THIRDPARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../thirdparty)

# Source files
set(SOURCES
    ${SRC_DIR}/android_main.cpp
    ${SRC_DIR}/shader.cpp
    ${SRC_DIR}/renderer.cpp
    ${SRC_DIR}/arc_renderer.cpp
    ${SRC_DIR}/polar_clock.cpp
    ${SRC_DIR}/text_renderer.cpp
    ${SRC_DIR}/asset_loader.cpp
    ${SRC_DIR}/platform/platform.cpp
    ${SRC_DIR}/platform/android_platform.cpp
)

# Create shared library (required for native activity)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}
    ${THIRDPARTY_DIR}
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Add native_app_glue source
add_library(native_app_glue STATIC
    ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)
target_include_directories(native_app_glue PUBLIC
    ${ANDROID_NDK}/sources/android/native_app_glue
)

# Find Android libraries
find_library(android-lib android)
find_library(log-lib log)
find_library(egl-lib EGL)
find_library(gles3-lib GLESv3)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    native_app_glue
    ${android-lib}
    ${log-lib}
    ${egl-lib}
    ${gles3-lib}
)

# Ensure ANativeActivity_onCreate is exported
set_target_properties(${PROJECT_NAME} PROPERTIES
    LINK_FLAGS "-u ANativeActivity_onCreate"
)
