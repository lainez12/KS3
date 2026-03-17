cmake_minimum_required(VERSION 3.20)


# Get version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/.version" RAW_VERSION)
string(STRIP "${RAW_VERSION}" CLEAN_VERSION)

# Set version to project
project(Kub3Software
    VERSION ${CLEAN_VERSION}
    LANGUAGES CXX
)

# Generate version file
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h.in"
    "${CMAKE_CURRENT_BINARY_DIR}/generated/version.h"
)

# ==============================================================================
# 1. CORE COMPILER SETTINGS (C++20)
# ==============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==============================================================================
# 2. QT 6.8 LTS SETUP (WIDGETS / UI)
# ==============================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "gui")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 6.8 REQUIRED COMPONENTS Core Gui Widgets SerialPort)
# find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Widgets SerialPort Network)

# ==============================================================================
# 3. CONDITIONAL COMPILATION (Opinionated Modularity)
# ==============================================================================
# This creates a drop-down in CMake GUIs and a configuration variable
set(KUB_MODEL "8" CACHE STRING "KUB3 Machine Model Target (4, 6, or 8)")
set_property(CACHE KUB_MODEL PROPERTY STRINGS "4" "6" "8")
message(STATUS "Configuring KUB3Software for model: KUB3-${KUB_MODEL}")

# ==============================================================================
# 4. HIKROBOT MVS LIBRARY (Industrial Vision)
# ==============================================================================
# Hikrobot MVS is typically installed in /opt/MVS on Linux.
# We use find_path and find_library so CMake gracefully handles missing dependencies.
find_path(MVS_INCLUDE_DIR
    NAMES MvCameraControl.h
    PATHS /opt/MVS/include $ENV{MVS_ROOT}/include
)

find_library(MVS_LIBRARY
    NAMES MvCameraControl
    PATHS /opt/MVS/lib/64 /opt/MVS/lib/32 $ENV{MVS_ROOT}/lib/64
)

if(MVS_INCLUDE_DIR AND MVS_LIBRARY)
    message(STATUS "Found MVS Hikrobot Library: ${MVS_LIBRARY}")
else()
    message(WARNING "MVS Hikrobot Library NOT FOUND! Camera features will be disabled or fail to compile.")
endif()

# ==============================================================================
# 5. PROJECT ARCHITECTURE & FILES
# ==============================================================================

set(PROJECT_SOURCES
    # Main & utils
    src/main.cpp
    src/utils.cpp
    include/utils.h
    # Layer 1: Views
    src/Views/MainWindow.cpp
    include/Views/MainWindow.h
    gui/MainWindow.ui
    # Layer 2: ViewModels
    # Layer 3: Master Finite State Machine (MFSM)
    # Layer 4: Services
    # Layer 5: Hardware Abstraction Layer (HAL)
)

qt_add_executable(Kub3Software
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

# ==============================================================================
# 6. INCLUDES & LINKING
# ==============================================================================

# Include directories
## Project
target_include_directories(Kub3Software PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}/generated"
    include
    include/HAL
    include/MFSM
    include/Services
    include/ViewModels
    include/Views
)
## MVS
if(MVS_INCLUDE_DIR)
    target_include_directories(Kub3Software PRIVATE ${MVS_INCLUDE_DIR})
endif()

# Inject the machine model into the C++ code as a preprocessor macro
target_compile_definitions(Kub3Software PRIVATE
    KUB_MODEL_${KUB_MODEL}
    KUB_MODEL_STR="${KUB_MODEL}"
)

# Link all together
## Project
target_link_libraries(Kub3Software PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::SerialPort
)
## MVS
if(MVS_LIBRARY)
    target_link_libraries(Kub3Software PRIVATE ${MVS_LIBRARY})
endif()
