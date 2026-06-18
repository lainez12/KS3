cmake_minimum_required(VERSION 3.20)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.version" RAW_VERSION)
string(STRIP "${RAW_VERSION}" CLEAN_VERSION)

project(Kub3Software VERSION ${CLEAN_VERSION} LANGUAGES CXX)

# Global Compiler Settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

# Define Machine Model globally
set(KUB_MODEL "8" CACHE STRING "KUB3 Machine Model Target (4, 6, or 8)")

# Find Global Dependencies
find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets SerialPort Svg)

find_path(MVS_INCLUDE_DIR NAMES MvCameraControl.h PATHS /opt/MVS/include $ENV{MVS_ROOT}/include)
find_library(MVS_LIBRARY NAMES MvCameraControl PATHS /opt/MVS/lib/64 /opt/MVS/lib/32 $ENV{MVS_ROOT}/lib/64)
if(NOT MVS_INCLUDE_DIR OR NOT MVS_LIBRARY)
    message(WARNING "MVS Hikrobot Library NOT FOUND. Vision features disabled.")
endif()

# Subdirectories
add_subdirectory(core)
add_subdirectory(apps/software)
add_subdirectory(apps/configurator)
add_subdirectory(apps/tester)
add_subdirectory(apps/config_generator)

# Tests
enable_testing()
add_subdirectory(tests)
