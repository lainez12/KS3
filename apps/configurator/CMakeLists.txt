# ==============================================================================
# CONFIGURATOR EXECUTABLE: Kub3Configurator
# ==============================================================================

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS "gui")

qt_add_executable(Kub3Configurator
    MANUAL_FINALIZATION
    # Sources
    src/main.cpp
    src/utils.cpp
    src/ConfigWindow.cpp
    src/security/AdminAuthenticator.cpp
    src/pages/AdminConfigPage.cpp
    src/pages/MotorConfigPage.cpp
    src/pages/CameraConfigPage.cpp
    src/pages/CameraGeneralPage.cpp
    src/pages/ForceConfigPage.cpp
    src/pages/ZAlgorithmicLimitsPage.cpp
    src/pages/DrawersPositionsPage.cpp
    src/pages/AlignmentPositionsPage.cpp
    src/pages/MCUConfigPage.cpp
    # Headers
    include/utils.h
    include/ConfigWindow.h
    include/security/AdminAuthenticator.h
    include/pages/AdminConfigPage.h
    include/pages/MotorConfigPage.h
    include/pages/CameraConfigPage.h
    include/pages/CameraGeneralPage.h
    include/pages/DrawersPositionsPage.h
    include/pages/ForceConfigPage.h
    include/pages/ZAlgorithmicLimitsPage.h
    include/pages/AlignmentPositionsPage.h
    include/pages/MCUConfigPage.h
    # UI
    gui/resources.qrc
    gui/ConfigWindow.ui
)

qt_finalize_target(Kub3Configurator)

target_include_directories(Kub3Configurator PRIVATE
    include
)

# Link against the main project's core library
target_link_libraries(Kub3Configurator PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(Kub3Configurator PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Include binary into the "bin/" dir of the output folder
install(TARGETS Kub3Configurator RUNTIME DESTINATION bin)

if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3Configurator
        OUTPUT_SCRIPT deploy_script_config
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script_config})
endif()
