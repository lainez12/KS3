# ==============================================================================
# MOTOR TESTER EXECUTABLE: Kub3Tester
# ==============================================================================
qt_add_executable(Kub3Tester
    MANUAL_FINALIZATION
    # Sources
    src/main.cpp
    src/controllers/ProcedureTestController.cpp
    src/controllers/MotorTestController.cpp
    src/controllers/CameraTestController.cpp
    src/ApplicationBuilder.cpp
    src/services/MotorTestService.cpp
    src/services/FocalTestService.cpp
    src/services/LightingTestService.cpp
    src/views/ViewBase.cpp
    src/views/MotorTestView.cpp
    src/views/ProcedureTestView.cpp
    src/views/CameraTestView.cpp
    src/views/components/RealTimeCurveWidget.cpp
    src/views/components/FocalTestModule.cpp
    src/views/components/CamLightTestModule.cpp
    src/TesterMainWindow.cpp
    src/view_models/ProcedureTestViewModel.cpp
    src/view_models/MotorTestViewModel.cpp
    src/view_models/CameraTestViewModel.cpp

    # Headers
    include/controllers/MotorTestController.h
    include/controllers/ProcedureTestController.h
    include/controllers/CameraTestController.h
    include/TesterMainWindow.h
    include/ApplicationBuilder.h
    include/services/MotorTestService.h
    include/services/FocalTestService.h
    include/services/LightingTestService.h
    include/views/ViewBase.h
    include/views/MotorTestView.h
    include/views/ProcedureTestView.h
    include/views/CameraTestView.h
    include/views/components/RealTimeCurveWidget.h
    include/views/components/FocalTestModule.h
    include/views/components/CamLightTestModule.h
    include/view_models/ProcedureTestViewModel.h
    include/view_models/MotorTestViewModel.h
    include/view_models/CameraTestViewModel.h

    # UI
    gui/resources.qrc
    gui/TesterMainWindow.ui
    gui/Views/ProcedureTestView.ui
    gui/Views/MotorTestView.ui
)

qt_finalize_target(Kub3Tester)

target_include_directories(Kub3Tester PRIVATE
    include
)

# Link against the main project's core library
target_link_libraries(Kub3Tester PRIVATE
    Kub3Core
    Qt6::Gui
    Qt6::Widgets
)

set_target_properties(Kub3Tester PROPERTIES
    INSTALL_RPATH "$ORIGIN;$ORIGIN/../lib;$ORIGIN/lib"
    BUILD_WITH_INSTALL_RPATH FALSE
)

# Include binary into the "bin/" dir of the output folder
install(TARGETS Kub3Tester RUNTIME DESTINATION bin)

if(TARGET Qt6::Core)
    qt_generate_deploy_app_script(
        TARGET Kub3Tester
        OUTPUT_SCRIPT deploy_script_config
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script_config})
endif()
