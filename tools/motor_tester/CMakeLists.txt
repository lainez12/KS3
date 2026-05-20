# ==============================================================================
# MOTOR TESTER EXECUTABLE: Kub3MotorTester
# ==============================================================================
qt_add_executable(Kub3MotorTester
    MANUAL_FINALIZATION
    # Sources
    src/main.cpp
    src/ApplicationBuilder.cpp
    src/MotorTestController.cpp
    src/MotorTestService.cpp
    src/MotorTestViewModel.cpp
    src/MotorTestWindow.cpp
    src/RealTimeCurveWidget.cpp
    # Headers
    include/ApplicationBuilder.h
    include/MotorTestController.h
    include/MotorTestService.h
    include/MotorTestViewModel.h
    include/MotorTestWindow.h
    include/RealTimeCurveWidget.h
    # UI
    gui/resources.qrc
    gui/MotorTestWindow.ui
)

qt_finalize_target(Kub3MotorTester)

target_include_directories(Kub3MotorTester PRIVATE
    include
)

# Link against the main project's core library
target_link_libraries(Kub3MotorTester PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(Kub3MotorTester PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Include binary into the "bin/" dir of the output folder
install(TARGETS Kub3MotorTester RUNTIME DESTINATION bin)

if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3MotorTester
        OUTPUT_SCRIPT deploy_script_config
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script_config})
endif()
